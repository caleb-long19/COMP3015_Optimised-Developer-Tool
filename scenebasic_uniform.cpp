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


// Switch Shaders on and off
bool switchShader_to_blinnPhongSpotlight = false;
bool switchShader_to_bpToonShader = false;
bool switchShader_to_phongDirectional = false;


//Material Uniform Floats (Used in render to change values via ImGui)
vec3 diffuseValue(0.4f, 0.4f, 0.4f);
vec3 specularValue(0.3f, 0.3f, 0.3f);
vec3 ambientValue(0.5f, 0.5f, 0.5f);


//Vecs used to change uniform values in the render method
vec3 spotDSValue(0.9f, 0.9f, 0.9f);
vec3 spotAmbientValue(1.0f, 1.0f, 1.0f);
vec3 fogColour(0.2f, 0.5f, 0.9f);


//Bool to turn music on or off
bool toggleCurrentMusic = true;


// Start the sound engine
ISoundEngine* backgoundMusic = createIrrKlangDevice();
ISound* music;


//Store users input from CIN at program launch
std::string shaderTypeInput;


SceneBasic_Uniform::SceneBasic_Uniform() : 
    angle(0.0f), 
    tPrev(0.0f),
    shadowMapWidth(1024), shadowMapHeight(1024),
    rotSpeed(glm::pi<float>() / 8.0f)
{
    //Custom Models
    streetMesh = ObjMesh::load("media/models/Street_Model.obj", true);
    yellowCarMesh = ObjMesh::load("media/models/Car_Yellow.obj", true);
    redCarMesh = ObjMesh::load("media/models/Car_Red.obj", true);
    lamp_postMesh = ObjMesh::load("media/models/Lamp_Post.obj", true);
    treeMesh = ObjMesh::load("media/models/Tree_Model.obj", true);
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

    //Enable Depth for 3D Rendering
    glEnable(GL_DEPTH_TEST);

    angle = glm::quarter_pi<float>();

    // Set up the framebuffer object
    setupFBO();

    GLuint programHandle = shader.getHandle();
    pass1Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "recordDepth");
    pass2Index = glGetSubroutineIndex(programHandle, GL_FRAGMENT_SHADER, "shadeWithShadow");

    shadowBias = mat4(vec4(0.5f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 0.5f, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 0.5f, 0.0f),
        vec4(0.5f, 0.5f, 0.5f, 1.0f)
    );

    float c = 1.65f;
    vec3 lightPos = vec3(0.0f, c * 5.25f, c * 7.5f);  // World coords
    lightFrustum.orient(lightPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    lightFrustum.setPerspective(50.0f, 1.0f, 1.0f, 25.0f);
    lightPV = shadowBias * lightFrustum.getProjectionMatrix() * lightFrustum.getViewMatrix();

    shader.setUniform("Light.Intensity", vec3(0.85f));
    shader.setUniform("ShadowMap", 0);

    //Load Textures (Diffuse/Normal or Diffuse)
    refreshShader();

    //Initialise the ImGUI for the Render Method
    ImGuiSetup();

    //Start The Music
    toggleMusic();
}


void SceneBasic_Uniform::spitOutDepthBuffer() {
    int size = shadowMapWidth * shadowMapHeight;
    float* buffer = new float[size];
    unsigned char* imgBuffer = new unsigned char[size * 4];

    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, buffer);

    for (int i = 0; i < shadowMapHeight; i++)
        for (int j = 0; j < shadowMapWidth; j++)
        {
            int imgIdx = 4 * ((i * shadowMapWidth) + j);
            int bufIdx = ((shadowMapHeight - i - 1) * shadowMapWidth) + j;

            // This is just to make a more visible image.  Scale so that
            // the range (minVal, 1.0) maps to (0.0, 1.0).  This probably should
            // be tweaked for different light configurations.
            float minVal = 0.88f;
            float scale = (buffer[bufIdx] - minVal) / (1.0f - minVal);
            unsigned char val = (unsigned char)(scale * 255);
            imgBuffer[imgIdx] = val;
            imgBuffer[imgIdx + 1] = val;
            imgBuffer[imgIdx + 2] = val;
            imgBuffer[imgIdx + 3] = 0xff;
        }

    delete[] buffer;
    delete[] imgBuffer;
    exit(1);
}

void SceneBasic_Uniform::setupFBO()
{
    GLfloat border[] = { 1.0f, 0.0f,0.0f,0.0f };
    // The depth buffer texture
    GLuint depthTex;
    glGenTextures(1, &depthTex);
    glBindTexture(GL_TEXTURE_2D, depthTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT24, shadowMapWidth, shadowMapHeight);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    // Assign the depth buffer texture to texture channel 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthTex);

    // Create and set up the FBO
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_2D, depthTex, 0);

    GLenum drawBuffers[] = { GL_NONE };
    glDrawBuffers(1, drawBuffers);

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
        shader.compileShader("shader/shadowMap.vert");
        shader.compileShader("shader/shadowMap.frag");
        shader.link();
        shader.use();

        // Used when rendering light frustum
        solidShader.compileShader("shader/solid.vert", GLSLShader::VERTEX);
        solidShader.compileShader("shader/solid.frag", GLSLShader::FRAGMENT);
        solidShader.link();
    }
    catch (GLSLProgramException& e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
    
}




void SceneBasic_Uniform::update(float t)
{
    //Update the angle position - used to animate the lighting object positions
    float deltaT = t - tPrev;
    if (tPrev == 0.0f) deltaT = 0.0f;
    tPrev = t;

    //Check to see if the m_animate bool is set to true - disable animation when false
    if (this->m_animate)
    {
        angle += 0.5f * deltaT;
        if (angle > glm::two_pi<float>()) 
            angle -= glm::two_pi<float>();
    }

}




void SceneBasic_Uniform::render()
{
    shader.use();

    // Generate the Shadow Map
    view = lightFrustum.getViewMatrix();
    projection = lightFrustum.getProjectionMatrix();

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, shadowMapWidth, shadowMapHeight);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass1Index);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.5f, 10.0f);

    drawScene();

    //Get Image From Depth Buffer
    glCullFace(GL_BACK);
    glFlush();

    // Render the shadow map
    float c = 2.0f;
    vec3 cameraPos(c * 8.0f * cos(angle), c * 6.0f, c * 4.5f * sin(angle));
    view = glm::lookAt(cameraPos, vec3(0.0f), vec3(0.0f, 1.0f, 0.0f));
    shader.setUniform("Light.Position", view * vec4(lightFrustum.getOrigin(), 1.0f));
    projection = glm::perspective(glm::radians(50.0f), (float)width / height, 0.1f, 100.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &pass2Index);

    drawScene();

    // Draw the light's frustum
    solidShader.use();
    solidShader.setUniform("Color", vec4(1.0f, 0.0f, 0.0f, 1.0f));
    mat4 mv = view * lightFrustum.getInverseViewMatrix();
    solidShader.setUniform("MVP", projection * mv);
    lightFrustum.render();
}

void SceneBasic_Uniform::drawScene()
{

    #pragma region Setup Material/Light Struct Uniforms - Can be changed via the ImGui
        //Setup Light Structure Uniforms - Mainly used for directional lighting - Diffuse/Specular Intensity - Ambient Intensity
        shader.setUniform("Light.L", spotDSValue);
        shader.setUniform("Light.La", spotAmbientValue);

        //Setup Spotlight Structure Uniforms - Adjust Diffuse/Specular Intensity - Adjust Ambient Intensity - Exponent Value - Cutoff (Distance of Spotlight)
        shader.setUniform("Spot.L", spotDSValue);
        shader.setUniform("Spot.La", spotAmbientValue);
        shader.setUniform("Spot.Exponent", spotExponentValue);
        shader.setUniform("Spot.Cutoff", glm::radians(spotCutoffValue));

        //Setup the Diffuse/Spectural/Ambient Material Reflective Power Uniforms
        shader.setUniform("Material.Kd", diffuseValue);
        shader.setUniform("Material.Ks", specularValue);
        shader.setUniform("Material.Ka", ambientValue);

        //Setup the Shininess (Glossy Look) of Spectural Material Uniform
        shader.setUniform("Material.Shininess", specularShininessValue);

        //Setup Fog Uniforms - Maximum Fog Distance - Minimum Fog Distance - Colour Of The Fog
        shader.setUniform("Fog.MaxDist", fogMaxDistanceValue);
        shader.setUniform("Fog.MinDist", fogMinDistanceValue);
        shader.setUniform("Fog.Color", fogColour);
    #pragma endregion



    #pragma region Load All Models - Assign Positions, Rotations and Scale


    #pragma region Street Model Settings

        // Alter the Poisition/Rotation/Size of the Street Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.0f, 5.0f, 1.0f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));

        //Load Matrices Settings & Render Street Mesh
        setMatrices();
        streetMesh->render();

    #pragma endregion


    #pragma region Car Model Settings
        // Alter the Poisition/Rotation/Size of the Yellow Car Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-0.4f, 4.75f, 1.0f));
        model = glm::rotate(model, glm::radians(87.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices();
        yellowCarMesh->render();


        // Alter the Poisition/Rotation/Size of the Red Car Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.3f, 4.75f, 2.6f));
        model = glm::rotate(model, glm::radians(-93.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Car Mesh
        setMatrices();
        redCarMesh->render();

    #pragma endregion


    #pragma region Lamp Models Settings

        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.05f, 5.3f, 1.2f));
        model = glm::rotate(model, glm::radians(90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices();
        lamp_postMesh->render();


        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.0f, 5.3f, -0.75f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices();
        lamp_postMesh->render();


        // Alter the Poisition/Rotation/Size of the Lamp Post Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(0.85f, 5.3f, 3.0f));
        model = glm::rotate(model, glm::radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.2f, 0.2f, 0.15f));

        //Load Matrices Settings & Render Lamp Post Mesh
        setMatrices();
        lamp_postMesh->render();

    #pragma endregion


    #pragma region Tree Models Settings

        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-3.0f, 5.0f, 0.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();

        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-1.5f, 5.0f, -1.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();

        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.6f, 5.0f, 1.7f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();

        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(-2.85f, 5.0f, 3.2f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(1.5f, 5.0f, 3.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.35f, 5.0f, 1.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.5f, 5.0f, 3.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();



        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(2.0f, 5.0f, 0.5f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();


        // Alter the Poisition/Rotation/Size of the Tree Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.0f, 5.0f, -1.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(0.15f, 0.15f, 0.15f));

        //Load Matrices Settings & Render Tree Mesh
        setMatrices();
        treeMesh->render();

        model = mat4(1.0f);
    #pragma endregion


    #pragma endregion



    #pragma region ImGUI Elements
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Display window with background colour and player speed changing values
        {
            ImGui::Begin("Dynamic Shader Representation Tool - Editor Window"); // Create Window - Title: Super Dodger

            //Checkbox to stop animation
            ImGui::Checkbox("Toggle Animation", &m_animate);

            // Create "gap" in gui to make a cleaner appearance
            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            //Button to turn music on or off
            if (ImGui::Button("Toggle Music"))
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

            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            //Change the intensity/colour values of the diffuse/specular & ambient uniforms for Lighting Structure
            ImGui::ColorEdit3("Diffuse/Specular Lighting Intensity: ", (float*)&spotDSValue);
            ImGui::ColorEdit3("Ambient Lighting Intensity: ", (float*)&spotAmbientValue);

            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            //Change the Spotlight Exponment & the cutoff distance uniforms for SpotLighting Structure
            ImGui::ColorEdit3("Spotlight Exponent: ", (float*)&spotExponentValue);
            ImGui::SliderFloat("Spotlight Cutoff: ", (float*)&spotCutoffValue, 1, 100);

            ImGui::Dummy(ImVec2(0.0f, 10.0f));

            //Change the intensity/colour values of the diffuse/specular/ambient for the material uniform structure
            ImGui::ColorEdit3("Diffuse Material Reflectivity: ", (float*)&diffuseValue);
            ImGui::ColorEdit3("Specular Material Reflectivity: ", (float*)&specularValue);
            ImGui::ColorEdit3("Ambient Material Reflectivity: ", (float*)&ambientValue);
            ImGui::SliderFloat("Specular Shininess Intensity: ", (float*)&specularShininessValue, 1, 200);

            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            //Change Fog Distance & Colours
            ImGui::SliderFloat("Fog Maximum Distance: ", (float*)&fogMaxDistanceValue, 0.1, 200);
            ImGui::SliderFloat("Fog Minimum Distance: ", (float*)&fogMinDistanceValue, 0.1, 200);
            ImGui::ColorEdit3("Fog Colour: ", (float*)&fogColour);

            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            //Button to exit application
            if (ImGui::Button("Exit Application"))
            {
                exit(EXIT_SUCCESS);
            }

            ImGui::Text("Average Framerate: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate); // Dispplay the framerate
            ImGui::End();
        }

        ImGui::Render();


        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    #pragma endregion

}


void SceneBasic_Uniform::setMatrices()
{
    #pragma region MyRegion
    //Create the Model View Matrix
    mat4 mv = view * model;

    //Set Model View Matrix Uniform
    shader.setUniform("ModelViewMatrix", mv);

    //Set the Normal Matrix Uniform
    shader.setUniform("NormalMatrix", glm::mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));

    //Multiply the Model View Matrix with the Projection Matrix
    shader.setUniform("MVP", projection * mv);
    shader.setUniform("ShadowMatrix", lightPV * model);
    #pragma endregion
}




void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(90.0f), (float)w / h, 0.3f, 100.0f);
}




void SceneBasic_Uniform::refreshShader()
{

   
    #pragma region Assign the textures

    // Load texture and bind it to the active meshes
    //GLuint texID = Texture::loadTexture("media/nice69-32x.png");
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, texID);

    #pragma endregion

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
