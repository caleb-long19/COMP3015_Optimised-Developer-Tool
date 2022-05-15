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


// Start the sound engine
ISoundEngine* backgroundSFX = createIrrKlangDevice();
ISound* forestAmbience;
ISound* carAmbience;


// Store the volume level of the ambient sounds
float forestVolume = 0.05f, carVolume = 0.1f;

// bool to toggle audio
bool toggleCurrentAmbience = true;

// Model Manipulation (Turn off rendering & alter positions)
float drivingDistance = 6.0f;
bool disablePlane = true;
bool creamHouseChimney = true, redHouseChimney = true, yellowHouseChimney = true, blueHouseChimney = true;
vec3 color;



int main(int argc, char* argv[])
{
    std::cout << "Input Your Desired Winodw Resolution: " << std::endl;
    std::cout << "Window Width: ";
    std::cin >> windowWidth;
    std::cout << "Window Height: ";
    std::cin >> windowHeight;

    //Run entire application
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
    vehicleSpeed(0.5f),
    nParticles(1500), 
    particleLifetime(5.0f),
    emitterPos(0.0f), 
    emitterDir(0, 1, 0),
    sky(100.0f)
{
    // Load The Town & House Meshes
    townMesh = ObjMesh::loadWithAdjacency("media/models/Street_Model.obj");
    whiteHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Normal.obj");
    yellowHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Yellow.obj");
    blueHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Blue.obj");
    redHouse = ObjMesh::loadWithAdjacency("media/models/House_Model_Red.obj");

    // Load The Tree, fence, lamp-post meshes
    fenceMesh = ObjMesh::loadWithAdjacency("media/models/Fence.obj");
    lamp_postMesh = ObjMesh::loadWithAdjacency("media/models/Lamp_Post.obj");
    treeMesh = ObjMesh::loadWithAdjacency("media/models/Tree_Model.obj");

    // Load The Vehicle Meshes
    yellowCarMesh = ObjMesh::loadWithAdjacency("media/models/Car_Yellow.obj");
    redCarMesh = ObjMesh::loadWithAdjacency("media/models/Car_Red.obj");
    planeDecay = ObjMesh::loadWithAdjacency("media/models/Plane.obj");
}



void SceneBasic_Uniform::initScene()
{
    // Load shader file, link it and activate the shader
    compile();

    // Clear Buffers
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClearStencil(0);
    
    //Enable Depth for 3D Rendering and colour blending (Supports the transparent effect)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);

    camAngle = 0.0f;

    // Setup Particles, Skybox/Cubemap & Geometry & Shadows
    setupParticles();
    setupSkybox();
    setupShadowVolumes();

    //Initialise the ImGUI for the Render Method
    ImGuiSetup();

    //Start The background ambience
    toggleAmbience();
}



void SceneBasic_Uniform::compile()
{
    try {
        // Locate & Link The Shader For The Volumes
        volumeShader.compileShader("shader/shadowVolume-vol.vert");
        volumeShader.compileShader("shader/shadowVolume-vol.geom");
        volumeShader.compileShader("shader/shadowVolume-vol.frag");
        volumeShader.link();

        // Locate & Link The Shader For Rendering and Compositing
        renderShader.compileShader("shader/shadowVolume-render.vert");
        renderShader.compileShader("shader/shadowVolume-render.frag");
        renderShader.link();

        // Locate & Link The Final Composite Shader
        compShader.compileShader("shader/shadowVolume-comp.vert");
        compShader.compileShader("shader/shadowVolume-comp.frag");
        compShader.link();

        // Locate & Link Skybox Shader
        skyShader.compileShader("shader/skybox.vert");
        skyShader.compileShader("shader/skybox.frag");
        skyShader.link();


        // Locate & Link Smoke Particle Shader
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
    deltaT = t - time;
    time = t;

    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;

    if (animating()) 
    { 
        // Adjust camera angles based on delta time and speed
        camAngle += deltaT * rotSpeed;

        // Adjust car angles based on delta time and speed
        vehicleAngle += deltaT * vehicleSpeed;
        if (camAngle > glm::two_pi<float>()) camAngle -= glm::two_pi<float>();
        updateLight();
    }
}



void SceneBasic_Uniform::updateLight()
{
    // Change the position of the directional light continuously
    // lightPos = vec4(150.0f * vec3(cosf(camAngle) * 0.5f, 1.5f, sinf(camAngle) * 4.5f), 1.0f);  // World coords
    lightPos = vec4(150.0f * vec3(cosf(camAngle) * lightPosX, lightPosY, sinf(camAngle) * lightPosZ), 1.0f);  // World coords

}



void SceneBasic_Uniform::render()
{
    pass1();
    glFlush();
    pass2();
    glFlush();
    pass3();
    glFinish();
}



void SceneBasic_Uniform::drawScene(GLSLProgram& shader, bool onlyShadowCasters)
{
    // Enable Depth for 3D Rendering
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // BlendFunc to blend rgba values | clear the colour and depth buffers
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Bind the textures and set the geometry shader uniform data
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, modelTex);
    color = vec3(1.0f);
    shader.setUniform("Ka", color * 0.1f);
    shader.setUniform("Kd", color);
    shader.setUniform("Ks", vec3(0.9f));
    shader.setUniform("Shininess", 150.0f);
    shader.setUniform("shaderType", shaderSwitch);
    

    #pragma region Load All Models - Assign Positions, Rotations and Scale

    #pragma region House Model Rendering & Model Data

        // Alter the Poisition / Rotation / Size of the Town Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, 5.0f, 1.0f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));
        setMatrices(shader);
        townMesh->render();

        // Alter the Poisition / Rotation / Size of the White House Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.0f, 5.37f, 2.35f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        whiteHouse->render();

        // Alter the Poisition / Rotation / Size of the Blue House Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-3.20f, 5.37f, 2.48f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        blueHouse->render();

        // Alter the Poisition / Rotation / Size of the Red House Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-3.15f, 5.37f, -0.60f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        yellowHouse->render();


        // Alter the Poisition / Rotation / Size of the Red House Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.25f, 5.37f, -0.3f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        redHouse->render();

        // Alter the Poisition / Rotation / Size of the Fence Meshes, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.72f, 5.1f, 1.0f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        fenceMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.67f, 5.1f, 1.0f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));
        setMatrices(shader);
        fenceMesh->render();


    #pragma endregion


    #pragma region Car Model Rendering & Model Data

        // Alter the Poisition / Rotation / Size of the Yellow Car Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-0.40f, 5.14f, drivingDistance * 0.25f * cos(vehicleAngle)));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        yellowCarMesh->render();

        // Alter the Poisition / Rotation / Size of the Red Car Mesh, set matrices/model data & render mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.45f, 5.14f, drivingDistance * 0.35f * sin(vehicleAngle)));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        redCarMesh->render();

    #pragma endregion


    #pragma region Lamp Models Rendering & Model Data

        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.25f, 5.8f, 1.2f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));
        setMatrices(shader);
        lamp_postMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.3f, 5.8f, -0.75f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));
        setMatrices(shader);
        lamp_postMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.1f, 5.8f, 3.0f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));
        setMatrices(shader);
        lamp_postMesh->render();

        if (disablePlane)
        {
            model = mat4(1.0f);
            model = glm::translate(model, vec3(5.0f * sin(vehicleAngle), 8.0f, 2.0f * cos(vehicleAngle)));
            model = glm::rotate(model, glm::radians(87.0f * cos(vehicleAngle)), vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, vec3(0.35f, 0.35f, 0.35f));

            // Set The Diffuse | Specular | Shininess Uniform Data in the Noise Shader
            setMatrices(shader);
            planeDecay->render();
        }

    #pragma endregion


    #pragma region Tree Models Rendering & Model Data

        // Alter the Poisition / Rotation / Size of the Tree Meshes, set matrices/model data & render mesh
        model = mat4(1.0f); 
        model = glm::translate(model, vec3(-3.0f, 5.0f, 0.4f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.5f, 5.0f, -1.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.6f, 5.0f, 1.7f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.85f, 5.0f, 3.2f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.5f, 5.0f, 3.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.35f, 5.0f, 1.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.5f, 5.0f, 3.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.0f, 5.0f, 0.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.0f, 5.0f, -1.4f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));
        setMatrices(shader);
        treeMesh->render();

    #pragma endregion

    #pragma endregion


    #pragma region Skybox

        // Rende The Custom Skybox
        skyShader.use();
        model = mat4(1.0f);
        setSkyboxMatrices(skyShader);
        sky.render();

    #pragma endregion


    #pragma region Particles Rendering & Model Data

        if (!onlyShadowCasters)
        {
            // Update pass
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

            // Smoke Particles for the white house chimney
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
            ImGui::Begin("The Town of Wakewood - Editor Window");       // Create Window - Title: The Town of Wakewood
            ImGui::Dummy(ImVec2(0.0f, 10.0f));                          // Create "gap" in gui to make a cleaner appearance


            ImGui::Text("Switch Shader Type");

            if (ImGui::Button("Phong Shading"))
            {
                if (shaderSwitch != 0)
                {
                    shaderSwitch = 0;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Toon Shading"))
            {
                if (shaderSwitch != 1)
                {
                    shaderSwitch = 1;
                }
            }

            ImGui::SameLine();

            if (ImGui::Button("Decay Shading"))
            {
                if (shaderSwitch != 2)
                {
                    shaderSwitch = 2;
                }
            }

            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Dummy(ImVec2(0.0f, 5.0f));   // Creates a blank space
            ImGui::Separator();                 // Creates a white line

            ImGui::Text("Model & Animation Controller");                                            // Section title
            ImGui::Checkbox("Toggle Plane", &disablePlane);                                         // Disable or Enable the animations in the scene
            ImGui::SliderFloat("Adjust All Animation Speeds", (float*)&rotSpeed, 0.1f, 1.0f);       // Slider that adjusts the speed of the animations in-game
            ImGui::SliderFloat("Adjust Vehicle Distance", (float*)&drivingDistance, 0.1f, 4.0f);    // Slider that adjusts driving distance of the cars
            ImGui::SliderFloat("Adjust Vehicle Speed", (float*)&vehicleSpeed, 0.1f, 6.0f);          // Slider that adjusts driving distance of the cars


            ImGui::Dummy(ImVec2(0.0f, 5.0f));   // Creates a blank space
            ImGui::Separator();                 // Creates a white line
            ImGui::Dummy(ImVec2(0.0f, 5.0f));


            ImGui::Text("Lighting Controller");                                                     // Section title
            ImGui::SliderFloat("Global Light Position - X", (float*)&lightPosX, 0.1f, 25.0f);       // Slider that adjusts the speed of the animations in-game
            ImGui::SliderFloat("Global Light Position - Y", (float*)&lightPosY, 0.1, 25.0f);        // Slider that adjusts driving distance of the cars
            ImGui::SliderFloat("Global Light Position - Z", (float*)&lightPosZ, 0.1, 25.0f);        // Slider that adjusts driving distance of the cars

            ImGui::Dummy(ImVec2(0.0f, 5.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 5.0f));


            ImGui::Text("House Customization");                                 // Section title
            ImGui::Checkbox("White House: Chimney", &creamHouseChimney);        // Disable White House Smoke
            ImGui::Checkbox("Yellow House: Chimney", &yellowHouseChimney);      // Disable Yellowe House Smoke
            ImGui::Checkbox("Red House: Chimney", &redHouseChimney);            // Disable Red House Smoke
            ImGui::Checkbox("Blue House: Chimney", &blueHouseChimney);          // Disable Blue House Smoke


            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 10.0f));


            ImGui::Text("Volume Settings");                                                        // Section title
            ImGui::SliderFloat("Forest Ambience - Volume", (float*)&forestVolume, 0.01f, 1.0f);    // Slider that adjusts the volume of the forest ambience
            ImGui::SliderFloat("Car Ambience - Volume", (float*)&carVolume, 0.01f, 0.5f);          // Slider that adjusts the volume of the car ambience
            if (ImGui::Button("Toggle Ambience"))                                                  // Button to turn forestAmbience on/off
            {
                if (toggleCurrentAmbience == true)
                {
                    //Turn forest Ambience off
                    toggleCurrentAmbience = false;
                    toggleAmbience();
                }
                else
                {
                    //Turn forest Ambience on
                    toggleCurrentAmbience = true;
                    toggleAmbience();
                }
            }


            ImGui::Dummy(ImVec2(0.0f, 10.0f));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0.0f, 10.0f));


            ImGui::Text("Tool Customization");                 // Section title
            ImGui::Checkbox("Toggle Animation", &m_animate);   // Disable or Enable the animations in the scene
            if (ImGui::Button("Exit Application"))             // IF user presses the exit button: close application
            {
                exit(EXIT_SUCCESS);
            }

            ImGui::Text("Average Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);       // Display the users verage framerate
            ImGui::End();

            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

            // Set volume
            forestAmbience->setVolume(forestVolume);
            carAmbience->setVolume(carVolume);

        #pragma endregion
}




#pragma region Buffer Setup Methods

void SceneBasic_Uniform::initBuffers() 
{
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

    // The diffuse + specular component
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

    // Set up the draw buffers so that we can write to the colour attachments
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result == GL_FRAMEBUFFER_COMPLETE) 
    {
        printf("Framebuffer is complete.\n");
    }
    else 
    {
        printf("Framebuffer is not complete.\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

#pragma endregion



#pragma region Setup/Render Geometry Shading, Shader Volumes, and Combine With Diffuse/Specular Lighting Methods 

// This renders the geometry normally with shading
// The ambient component is rendered to one buffer, and the diffuse and specular componenets are written to a texture.
void SceneBasic_Uniform::pass1() 
{
    glDepthMask(GL_TRUE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);

    float c = 2.0f;

    // Setup the camera projectio and position | Camera position is animation using camAngle 
    projection = glm::infinitePerspective(glm::radians(50.0f), (float)width / height, 0.5f);
    vec3 cameraPos(c * 2.0f * cos(camAngle), c * 6.0f, c * 4.5f * sin(camAngle));
    view = glm::lookAt(cameraPos, vec3(0, 6, 0), vec3(0, 1, 0));


    // Load The Shader For Rendering and Compositing | Set Uniform Data To Calculate The Position Of The Light
    renderShader.use();
    renderShader.setUniform("LightPosition", view * lightPos);


    // Bind Framebuffer | Clear Depth & Colour Buffers
    glBindFramebuffer(GL_FRAMEBUFFER, colorDepthFBO);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


    // Render The Sceen
    drawScene(renderShader, false);
}



// This is the pass that generates the shadow volumes using the geometry shader
void SceneBasic_Uniform::pass2() 
{

    // Load The Shader For The Volumes | Set Uniform Data To Calculate The Position Of The Light
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

    // Setup The stencil test so that it always succeeds, increments for front faces, and decrements for back faces.
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



// In this pass, we read the diffuse and specular component from a texture and combine it with the ambient component if the stencil test succeeds.
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

#pragma endregion



#pragma region Setup Shaders/Techniques Methods

// Setup the particle buffers, textures & uniform data
void SceneBasic_Uniform::setupParticles() 
{
    model = mat4(1.0f);

    // Initiate the particle buffers
    initBuffers();

    // Load the smoke texture
    glActiveTexture(GL_TEXTURE3);
    Texture::loadTexture("media/texture/smoke.png");

    // Load randomly generated particles
    glActiveTexture(GL_TEXTURE4);
    ParticleUtils::createRandomTex1D(nParticles * 3);

    // Activate the particle shader and set the uniform data
    smokeShader.use();
    smokeShader.setUniform("ParticleTex", 3);                       // Assign Texture ID
    smokeShader.setUniform("ParticleLifetime", particleLifetime);   // Time Particles Are Alive 
    smokeShader.setUniform("Accel", vec3(0.0f, 0.1f, 0.0f));        // Particle Acceleration
    smokeShader.setUniform("MinParticleSize", float(0.05));         // Minimum Particle Size
    smokeShader.setUniform("MaxParticleSize", float(0.2));          // Maximum Particle Size
    smokeShader.setUniform("RandomTex", 4);                         // Assign Random Texture ID

    // Set Particle Emiitter Position & Direction
    smokeShader.setUniform("Emitter", emitterPos);
    smokeShader.setUniform("EmitterBasis", ParticleUtils::makeArbitraryBasis(emitterDir));
}



// Load the Skybox data
void SceneBasic_Uniform::setupSkybox() 
{
    // Find Textures and bind them to the cube map (Skybox)
    GLuint cubeTex = Texture::loadHdrCubeMap("media/skybox/sky-hdr/sky");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTex);
}



// Load shadow volume/geomtry shaders
void SceneBasic_Uniform::setupShadowVolumes() 
{
    setupFBO();

    renderShader.use();
    renderShader.setUniform("LightIntensity", vec3(1.0f));

    // Set up a  VAO for the full-screen quad
    GLfloat verts[] = 
    {
      -1.0f, -1.0f, 0.0f, 1.0f, 
      -1.0f,  0.0f, 1.0f, 1.0f, 
       0.0f, -1.0f, 1.0f, 0.0f 
    };

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

    //Load texture/s
    glActiveTexture(GL_TEXTURE2);
    modelTex = Texture::loadTexture("media/nice69-32x.png");

    updateLight();

    // Bind The Generated Noise Texture
    GLuint noiseTex = NoiseTex::generate2DTex(20.0f);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, noiseTex);

    renderShader.use();                                 // Activate The Shadow Volume Render Shader
    renderShader.setUniform("Tex", 2);                  // Set Texture Uniorm For Rendering
    renderShader.setUniform("NoiseTex", 5);             // Set the randomly generated noise texture
    renderShader.setUniform("LowThreshold", 0.45f);     // Low noise threshold
    renderShader.setUniform("HighThreshold", 0.65f);    // High noise threshold
    renderShader.setUniform("NoiseTex", 5);             // Set Texture Uniorm For The Noise Shader

    compShader.use();                           // Activate The Shadow Volume Composition Shader
    compShader.setUniform("DiffSpecTex", 0);    // Assign Loaded Texture ID
}

#pragma endregion



#pragma region Set Matrices Methods

// Set Matrices for the geometry/shadow shaders
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


#pragma endregion



#pragma region External Code - forestAmbience - Window Size - ImGUI


void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
}



void SceneBasic_Uniform::toggleAmbience()
{
    if (toggleCurrentAmbience == true) 
    {
        // Loop background Ambience
        forestAmbience = backgroundSFX->play2D("media/audio/Forest_Ambience.mp3", true, false, true);
        carAmbience = backgroundSFX->play2D("media/audio/Cars_DrivingSFX.mp3", true, false, true);
    }
    else 
    {
        //Stop playing the ambience
        forestAmbience->stop();
        carAmbience->stop();
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