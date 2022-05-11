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


//Bool to turn music on or off
bool toggleCurrentMusic = true;


// Start the sound engine
ISoundEngine* backgoundMusic = createIrrKlangDevice();
ISound* music;


//Store users input from CIN at shaderram launch
std::string shaderTypeInput;





SceneBasic_Uniform::SceneBasic_Uniform() : 
    tPrev(0),
    rotSpeed(0.1f)
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



int main(int argc, char* argv[])
{
    //Run entire application when user has selected shader e.g. load window, shaders, uniform data, etc.
    SceneRunner runner("The Street Of Wakewood");

    std::unique_ptr<Scene> scene;

    scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());

    return runner.run(*scene);
}




void SceneBasic_Uniform::initScene()
{
    // Load shader file, link it and activate the shader
    compile();

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClearStencil(0);

    //Enable Depth for 3D Rendering
    glEnable(GL_DEPTH_TEST);

    angle = 0.0f;

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

    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}







void SceneBasic_Uniform::update(float t)
{
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;
    if (animating()) {
        angle += deltaT * rotSpeed;
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
    vec3 color;

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

        #pragma endregion


        #pragma region Car Model Settings

            // Alter the Poisition/Rotation/Size of the Yellow Car Mesh
            model = mat4(1.0f);
            model = glm::translate(model, vec3(-0.4f, 5.0f, 1.0f));
            model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
            model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

            //Load Matrices Settings & Render Car Mesh
            setMatrices(shader);
            yellowCarMesh->render();


            // Alter the Poisition/Rotation/Size of the Red Car Mesh
            model = mat4(1.0f);
            model = glm::translate(model, vec3(0.3f, 5.0f, 2.6f));
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
            model = glm::translate(model, vec3(-3.0f, 5.0f, 0.8f));
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

            model = mat4(1.0f);
        #pragma endregion



    #pragma endregion

}




//void SceneBasic_Uniform::drawScene(GLSLProgram &shader, bool onlyShadowCasters)
//{
//    vec3 color;
//
//    if (!onlyShadowCasters) {
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, spotTex);
//        color = vec3(1.0f);
//        shader.setUniform("Ka", color * 0.1f);
//        shader.setUniform("Kd", color);
//        shader.setUniform("Ks", vec3(0.9f));
//        shader.setUniform("Shininess", 150.0f);
//    }
//
//    model = mat4(1.0f);
//    model = glm::translate(model, vec3(-2.3f, 1.0f, 0.2f));
//    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
//    model = glm::scale(model, vec3(1.5f));
//    setMatrices(shader);
//    spot->render();
//
//    model = mat4(1.0f);
//    model = glm::translate(model, vec3(2.5f, 1.0f, -1.2f));
//    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
//    model = glm::scale(model, vec3(1.5f));
//    setMatrices(shader);
//    spot->render();
//
//    model = mat4(1.0f);
//    model = glm::translate(model, vec3(0.5f, 1.0f, 2.7f));
//    model = glm::rotate(model, glm::radians(180.0f), vec3(0.0f, 1.0f, 0.0f));
//    model = glm::scale(model, vec3(1.5f));
//    setMatrices(shader);
//    spot->render();
//
//    if (!onlyShadowCasters) {
//        glActiveTexture(GL_TEXTURE2);
//        glBindTexture(GL_TEXTURE_2D, brickTex);
//        color = vec3(0.5f);
//        shader.setUniform("Kd", color);
//        shader.setUniform("Ks", vec3(0.0f));
//        shader.setUniform("Ka", vec3(0.1f));
//        shader.setUniform("Shininess", 1.0f);
//        model = mat4(1.0f);
//        setMatrices(shader);
//        plane.render();
//        model = mat4(1.0f);
//        model = glm::translate(model, vec3(-5.0f, 5.0f, 0.0f));
//        model = glm::rotate(model, glm::radians(90.0f), vec3(1, 0, 0));
//        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
//        setMatrices(shader);
//        plane.render();
//        model = mat4(1.0f);
//        model = glm::translate(model, vec3(0.0f, 5.0f, -5.0f));
//        model = glm::rotate(model, glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));
//        setMatrices(shader);
//        plane.render();
//        model = mat4(1.0f);
//    }
//       
//}




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
    // We don't need the depth test
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







void SceneBasic_Uniform::setMatrices(GLSLProgram &shader)
{
    mat4 mv = view * model;
    shader.setUniform("ModelViewMatrix", mv);
    shader.setUniform("ProjMatrix", projection);
    shader.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
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