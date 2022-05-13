#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"

#include <sstream>
#include <iostream>
using std::cerr;
using std::endl;

#include <glm/gtc/matrix_transform.hpp>
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;


// Fog Colours
float drivingDistance = 6.0f;


// Start the sound engine & Bool to turn music on or off
ISoundEngine* backgoundMusic = createIrrKlangDevice();
ISound* music;
bool toggleCurrentMusic = true;



int main(int argc, char* argv[])
{
    //Run entire application when user has selected shader e.g. load window, shaders, uniform data, etc.
    SceneRunner runner("The Town Of Wakewood");

    std::unique_ptr<Scene> scene;

    scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());

    return runner.run(*scene);
}




SceneBasic_Uniform::SceneBasic_Uniform() :
    tPrev(0),
    drawBuf(1),
    time(0),
    deltaT(0),
    rotSpeed(0.5f),
    carSpeed(0.5f),
    sky(100.0f),
    nParticles(1500),
    particleLifetime(5.0f),
    emitterPos(0.0f),
    emitterDir(0, 1, 0)
{
    //Custom Models
    streetMesh = ObjMesh::loadWithAdjacency("media/models/Street_Model.obj");
    defaultHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Normal.obj");
    yellowHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Yellow.obj");
    blueHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Blue.obj");
    redHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Red.obj");


    fenceMesh = ObjMesh::loadWithAdjacency("media/models/Fence.obj");
    lamp_postMesh = ObjMesh::loadWithAdjacency("media/models/Lamp_Post.obj");
    treeMesh = ObjMesh::loadWithAdjacency("media/models/Tree_Model.obj");


    yellowCarMesh = ObjMesh::loadWithAdjacency("media/models/Car_Yellow.obj");
    redCarMesh = ObjMesh::loadWithAdjacency("media/models/Car_Red.obj");
}



void SceneBasic_Uniform::initScene()
{
    // Load shader file, link it and activate the shader
    compile();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearStencil(0);
    
    //Enable Depth for 3D Rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    //angle = 0.0f;

    model = mat4(1.0f);

    initBuffers();

    glActiveTexture(GL_TEXTURE3);
    Texture::loadTexture("media/texture/smoke.png");

    glActiveTexture(GL_TEXTURE4);
    ParticleUtils::createRandomTex1D(nParticles * 3);

    smokeShader.use();
    smokeShader.setUniform("ParticleTex", 3);
    smokeShader.setUniform("ParticleLifetime", particleLifetime);
    smokeShader.setUniform("Accel", vec3(0.0f, 0.1f, 0.0f));
    smokeShader.setUniform("MinParticleSize", float(0.05));
    smokeShader.setUniform("MaxParticleSize", float(0.2));
    smokeShader.setUniform("RandomTex", 4);
    smokeShader.setUniform("Emitter", emitterPos);
    smokeShader.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));


    // Find Textures and bind them to the cube map (Skybox)
    GLuint cubeTex = Texture::loadHdrCubeMap("media/skybox/sky-hdr/sky");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);

    setupFBO();

    renderShader.use();
    renderShader.setUniform("LightIntensity", vec3(1.0f));

    // Set up a  VAO for the full-screen quad
    GLfloat verts[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
                    1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f };

    GLuint bufHandle;
    glGenBuffers(1, &bufHandle);
    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(GLfloat), verts, GL_STATIC_DRAW);

    // Set up the vertex array object
    glGenVertexArrays(1, &fsQuad);
    glBindVertexArray(fsQuad);

    glBindBuffer(GL_ARRAY_BUFFER, bufHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindVertexArray(0);

    //Load textures
    glActiveTexture(GL_TEXTURE2);
    spotTex = Texture::loadTexture("media/nice69-32x.png");
    brickTex = Texture::loadTexture("media/nice69-32x.png");

    updateLight();

    renderShader.use();
    renderShader.setUniform("Tex", 2);

    compShader.use();
    compShader.setUniform("DiffSpecTex", 0);


    this->animate(true);


    //Initialise the ImGUI for the Render Method
    ImGuiSetup();

    //Start The Music
    toggleMusic();
}



void SceneBasic_Uniform::updateLight()
{
    lightPos = vec4(150.0f * vec3(cosf(angle) * 0.5f, 1.5f, sinf(angle) * 4.5f), 1.0f);  // World coords
}



void SceneBasic_Uniform::compile()
{
    try {
        // The shader for the volumes
        volumeShader.compileShader("shader/shadowVolume-vol.vert");
        volumeShader.compileShader("shader/shadowVolume-vol.geom");
        volumeShader.compileShader("shader/shadowVolume-vol.frag");
        volumeShader.link();

        // The shader for rendering and compositing
        renderShader.compileShader("shader/shadowVolume-render.vert");
        renderShader.compileShader("shader/shadowVolume-render.frag");
        renderShader.link();

        // The final composite shader
        compShader.compileShader("shader/shadowVolume-comp.vert");
        compShader.compileShader("shader/shadowVolume-comp.frag");
        compShader.link();

        // Skybox Shader
        skyShader.compileShader("shader/skybox.vert");
        skyShader.compileShader("shader/skybox.frag");
        skyShader.link();


        // Smoke Particle Shader
        smokeShader.compileShader("shader/smokeParticles.vert");
        smokeShader.compileShader("shader/smokeParticles.frag");

        GLuint shaderHandle = smokeShader.getHandle();
        const char* outputNames[] = { "Position", "Velocity", "Age" };
        glTransformFeedbackVaryings(shaderHandle, 3, outputNames, GL_SEPARATE_ATTRIBS);

        smokeShader.link();
        smokeShader.use();

    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}



void SceneBasic_Uniform::update(float t)
{
    float deltaTime = t - tPrev;

    deltaT = t - time;
    time = t;

    // Particles
    if (tPrev == 0.0f) deltaTime = 0.0f;
    tPrev = t;

    if (animating()) 
    { 
        // Car and Camera Angles
        angle += deltaTime * rotSpeed;
        carAngle += deltaTime * carSpeed;
        if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
        updateLight();
    }
}



void SceneBasic_Uniform::render()
{
    pass1();
    glFlush();
    pass2();
    glFlush();
    pass3();
}



void SceneBasic_Uniform::drawScene(GLSLProgram& shader, bool onlyShadowCasters)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    vec3 color;

    //Enable Depth for 3D Rendering
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!onlyShadowCasters) {
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, spotTex);
        color = vec3(1.0f);
        shader.setUniform("Ka", color * 0.1f);
        shader.setUniform("Kd", color);
        shader.setUniform("Ks", vec3(0.9f));
        shader.setUniform("Shininess", 150.0f);

    }

    #pragma region Load All Models - Assign Positions, Rotations and Scale


    #pragma region House Model Settings

        // Alter the Poisition/Rotation/Size of the Yellow Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(3.0f, 5.37f, 2.35f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        defaultHouse->render();



        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(-3.20f, 5.37f, 2.48f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        blueHouse->render();



        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(-3.15f, 5.37f, -0.60f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        yellowHouse->render();



        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(3.25f, 5.37f, -0.3f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        redHouse->render();



        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(2.72f, 5.1f, 1.0f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        fenceMesh->render();



        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(-2.67f, 5.1f, 1.0f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        fenceMesh->render();


    #pragma endregion


    #pragma region Car Model Settings

        // Alter the Poisition/Rotation/Size of the Yellow Car Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(-0.40f, 5.14f, drivingDistance * 0.25f * cos(carAngle)));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        yellowCarMesh->render();




        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.45f, 5.14f, drivingDistance * 0.35f * sin(carAngle)));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices(shader);
        redCarMesh->render();




        // Alter the Poisition/Rotation/Size of the Street Mesh
        model = mat4(1.0f);

        model = glm::translate(model, vec3(0.0f, 5.0f, 1.0f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));


        setMatrices(shader);
        streetMesh->render();

    #pragma endregion


    #pragma region Lamp Models Settings

        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.25f, 5.8f, 1.2f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices(shader);
        lamp_postMesh->render();



        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.3f, 5.8f, -0.75f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices(shader);
        lamp_postMesh->render();



        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.1f, 5.8f, 3.0f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices(shader);
        lamp_postMesh->render();

    #pragma endregion


    #pragma region Tree Models Settings

        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-3.0f, 5.0f, 0.4f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.5f, 5.0f, -1.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.6f, 5.0f, 1.7f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.85f, 5.0f, 3.2f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.5f, 5.0f, 3.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.35f, 5.0f, 1.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.5f, 5.0f, 3.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.0f, 5.0f, 0.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();


        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.0f, 5.0f, -1.4f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices(shader);
        treeMesh->render();
    #pragma endregion


    #pragma endregion


    // Draw skybox
    skyShader.use();
    model = mat4(1.0f);
    setSkyboxMatrices(skyShader);
    sky.render();

    #pragma region Particles

    if (!onlyShadowCasters) {

        //Update pass
        if (animating())
        {
            smokeShader.use();
            smokeShader.setUniform("Pass", 1);
            smokeShader.setUniform("Time", time);
            smokeShader.setUniform("DeltaT", deltaT);

            glEnable(GL_RASTERIZER_DISCARD);
            glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[drawBuf]);
            glBeginTransformFeedback(GL_POINTS);
            glBindVertexArray(particleArray[1 - drawBuf]);
            glVertexAttribDivisor(0, 0);
            glVertexAttribDivisor(1, 0);
            glVertexAttribDivisor(2, 0);
            glDrawArrays(GL_POINTS, 0, nParticles);
            glEndTransformFeedback();
            glDisable(GL_RASTERIZER_DISCARD);

            // Render pass
            smokeShader.setUniform("Pass", 2);
        }

        // Smoke Particles for the default house chimney
        if (creamHouseChimney)
        {
            model = mat4(1.0f);
            model = glm::translate(model, vec3(3.5f, 5.9f, 2.35f));
            smokeShader.use();
            setParticleMatrices(smokeShader);
            glDepthMask(GL_FALSE);
            glBindVertexArray(particleArray[drawBuf]);
            glVertexAttribDivisor(0, 1);
            glVertexAttribDivisor(1, 1);
            glVertexAttribDivisor(2, 1);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
        }



        // Smoke Particles for the red house chimney
        if (redHouseChimney)
        {
            model = mat4(1.0f);
            model = glm::translate(model, vec3(3.7f, 5.9f, -0.3f));
            smokeShader.use();
            setParticleMatrices(smokeShader);
            glDepthMask(GL_FALSE);
            glBindVertexArray(particleArray[drawBuf]);
            glVertexAttribDivisor(0, 1);
            glVertexAttribDivisor(1, 1);
            glVertexAttribDivisor(2, 1);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
        }



        // Smoke Particles for the yellow house chimney
        if (yellowHouseChimney)
        {
            model = mat4(1.0f);
            model = glm::translate(model, vec3(-3.53f, 5.9f, -0.60f));
            smokeShader.use();
            setParticleMatrices(smokeShader);
            glDepthMask(GL_FALSE);
            glBindVertexArray(particleArray[drawBuf]);
            glVertexAttribDivisor(0, 1);
            glVertexAttribDivisor(1, 1);
            glVertexAttribDivisor(2, 1);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
        }



        // Smoke Particles for the blue house chimney
        if (blueHouseChimney)
        {
            model = mat4(1.0f);
            model = glm::translate(model, vec3(-3.53f, 5.9f, 2.48f));
            smokeShader.use();
            setParticleMatrices(smokeShader);
            glDepthMask(GL_FALSE);
            glBindVertexArray(particleArray[drawBuf]);
            glVertexAttribDivisor(0, 1);
            glVertexAttribDivisor(1, 1);
            glVertexAttribDivisor(2, 1);
            glDrawArraysInstanced(GL_TRIANGLES, 0, 6, nParticles);
            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
        }



        //// Swap buffers
        drawBuf = 1 - drawBuf;
    }
    
    #pragma endregion


    #pragma region ImGUI Elements

                    // Start the Dear ImGui frame
                    ImGui_ImplOpenGL3_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();


                    // Display window with background colour and player speed changing values
                    {
                        ImGui::Begin("The Town of Wakewood - Editor Window");                                    // Create Window - Title: The Town of Wakewood


                        ImGui::Dummy(ImVec2(0.0f, 10.0f));                                                       // Create "gap" in gui to make a cleaner appearance


                        ImGui::Text("Animation Controller");
                        ImGui::SliderFloat("Adjust All Animation Speeds", (float*)&rotSpeed, 0.1, 1);            // Slider that adjusts the speed of the animations in-game
                        ImGui::SliderFloat("Adjust Vehicle Distance", (float*)&drivingDistance, 0.1, 4.0f);      // Slider that adjusts driving distance of the cars
                        ImGui::SliderFloat("Adjust Vehicle Speed", (float*)&carSpeed, 0.1, 6.0f);                // Slider that adjusts driving distance of the cars


                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 10.0f));


                        //Checkbox to stop chimney smoke
                        ImGui::Text("House Customization");
                        ImGui::Checkbox("Cream House: Chimney", &creamHouseChimney);
                        ImGui::Checkbox("Yellow House: Chimney", &yellowHouseChimney);
                        ImGui::Checkbox("Red House: Chimney", &redHouseChimney);
                        ImGui::Checkbox("Blue House: Chimney", &blueHouseChimney);


                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 10.0f));


                        ImGui::Text("Tool Customization");
                        if (ImGui::Button("Toggle Music"))                      //Button to turn music on or off
                        {
                            if (toggleCurrentMusic == true)
                            {
                                //Turn music off
                                toggleCurrentMusic = false;
                                toggleMusic();
                            }
                            else
                            {
                                //Turn music on
                                toggleCurrentMusic = true;
                                toggleMusic();
                            }
                        }
                        ImGui::Checkbox("Toggle Animation", &m_animate);        // Disable or Enable the animations in the scene


                        ImGui::Dummy(ImVec2(0.0f, 10.0f));
                        ImGui::Separator();
                        ImGui::Dummy(ImVec2(0.0f, 10.0f));


                        if (ImGui::Button("Exit Application"))  //Button to exit application
                        {
                            exit(EXIT_SUCCESS);
                        }


                        ImGui::Text("Average Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); // Display the framerate
                        ImGui::End();

                        }

                        ImGui::Render();
                        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

        #pragma endregion
}



void SceneBasic_Uniform::initBuffers() {
    // Generate the buffers
    glGenBuffers(2, posBuf);    // position buffers
    glGenBuffers(2, velBuf);    // velocity buffers
    glGenBuffers(2, age);       // Start time buffers

    // Allocate space for all buffers
    int size = nParticles * 3 * sizeof(GLfloat);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(GLfloat), 0, GL_DYNAMIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glBufferData(GL_ARRAY_BUFFER, nParticles * sizeof(GLfloat), 0, GL_DYNAMIC_COPY);

    // Fill the first age buffer
    std::vector<GLfloat> initialAge(nParticles);
    float rate = particleLifetime / nParticles;
    for (int i = 0; i < nParticles; i++) initialAge[i] = rate * (i - nParticles);
    Random::shuffle(initialAge);  // Shuffle ages for more uniformity
    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, nParticles * sizeof(GLfloat), initialAge.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create vertex arrays for each set of buffers
    glGenVertexArrays(2, particleArray);

    // Set up particle array 0
    glBindVertexArray(particleArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[0]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[0]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[0]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    // Set up particle array 1
    glBindVertexArray(particleArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, posBuf[1]);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, velBuf[1]);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, age[1]);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    // Setup the feedback objects
    glGenTransformFeedbacks(2, feedback);

    // Transform feedback 0
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[0]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[0]);

    // Transform feedback 1
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, feedback[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, posBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, velBuf[1]);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, age[1]);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
}



void SceneBasic_Uniform::setupFBO()
{
    // The depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // The ambient buffer
    GLuint ambBuf;
    glGenRenderbuffers(1, &ambBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, ambBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);

    // The diffuse+specular component
    glActiveTexture(GL_TEXTURE0);
    GLuint diffSpecTex;
    glGenTextures(1, &diffSpecTex);
    glBindTexture(GL_TEXTURE_2D, diffSpecTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create and set up the FBO
    glGenFramebuffers(1, &colorDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, ambBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, diffSpecTex, 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is complete.\n");
    }
    else {
        printf("Framebuffer is not complete.\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}







// Just renders the geometry normally with shading.The ambient component
// is rendered to one buffer, and the diffuse and specular componenets are
// written to a texture.
void SceneBasic_Uniform::pass1() {
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    float c = 2.0f;

    projection = glm::infinitePerspective(glm::radians(50.0f), (float)width / height, 0.5f);
    vec3 cameraPos(c * 2.0f * cos(angle), c * 6.0f, c * 4.5f * sin(angle));
    view = glm::lookAt(cameraPos, vec3(0, 6, 0), vec3(0, 1, 0));

    renderShader.use();
    renderShader.setUniform("LightPosition", view * lightPos);

    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    drawScene(renderShader, false);
}



// This is the pass that generates the shadow volumes using the
// geometry shader
void SceneBasic_Uniform::pass2() {
    volumeShader.use();
    volumeShader.setUniform("LightPosition", view * lightPos);

    // Copy the depth and color buffers from the FBO into the default FBO
    // The color buffer should contain the ambient component.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, colorDepthFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width - 1, height - 1, 0, 0, width - 1, height - 1, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

    // Disable writing to the color buffer and depth buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    // Re-bind to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Set up the stencil test so that it always succeeds, increments
    // for front faces, and decrements for back faces.
    glClear(GL_STENCIL_BUFFER_BIT);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0xffff);
    glStencilOpSeparate(GL_FRONT, GL_KEEP, GL_KEEP, GL_INCR_WRAP);
    glStencilOpSeparate(GL_BACK, GL_KEEP, GL_KEEP, GL_DECR_WRAP);

    // Draw only the shadow casters
    drawScene(volumeShader, true);

    // Enable writing to the color buffer
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}



// In this pass, we read the diffuse and specular component from a texture
// and combine it with the ambient component if the stencil test succeeds.
void SceneBasic_Uniform::pass3() {

    // Disable the depth test
    glDisable(GL_DEPTH_TEST);

    // We want to just sum the ambient component and the diffuse + specular
    // when the stencil test succeeds, so we'll use this simple blend function.
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    // We want to only render those pixels that have a stencil value
    // equal to zero.
    glStencilFunc(GL_EQUAL, 0, 0xffff);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

    compShader.use();

    // Just draw a screen filling quad
    model = mat4(1.0f);
    projection = model;
    view = model;
    setMatrices(compShader);

    glBindVertexArray(fsQuad);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0);

    // Restore some state
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}





// Set Matrices for the default shaders
void SceneBasic_Uniform::setMatrices(GLSLProgram &shader)
{
    mat4 mv = view * model;

    shader.setUniform("ModelViewMatrix", mv);
    shader.setUniform("ProjMatrix", projection);
    shader.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

}


// Set Matrices for the skybox
void SceneBasic_Uniform::setSkyboxMatrices(GLSLProgram& skyShader)
{
    mat4 mv = view * model;
    skyShader.setUniform("ModelMatrix", model);
    skyShader.setUniform("MVP", projection * mv);
}


// Set Matrices for the Particles
void SceneBasic_Uniform::setParticleMatrices(GLSLProgram& particleShader)
{
    mat4 mv = view * model;
    particleShader.setUniform("Proj", projection);
    particleShader.setUniform("MV", mv);
}





#pragma region External Code - Music - Window Size - ImGUI


void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}





void SceneBasic_Uniform::toggleMusic()
{
    if (toggleCurrentMusic == true) 
    {
        // Loop background music in game and set the volume
        music = backgoundMusic->play2D("media/audio/Art Of Silence - Uniq.mp3", true, false, true);
        music->setVolume(0.07f);
    }
    else 
    {
        //Stop playing the music
        music->stop();
    }

}






void SceneBasic_Uniform::ImGuiSetup()
{
    // Initialising ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

}



#pragma endregion