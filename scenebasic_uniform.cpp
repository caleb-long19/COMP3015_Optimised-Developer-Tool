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
    rotSpeed(glm::pi<float>() / 8.0f)
{
    //Custom Models
    houseMesh = ObjMesh::load("media/models/House_Model.obj", true);
    islandMesh = ObjMesh::load("media/models/Island_Model.obj", true);
    treeMesh = ObjMesh::load("media/models/Tree_Model.obj", true);

    //Third-Party Models
    ogre = ObjMesh::load("media/bs_ears.obj", false, true);
}


int main(int argc, char* argv[])
{

    //CIN/COUT Introduction in CMD
    std::cout << "PLEASE CHOOSE THE SHADER TYPE YOU WISH TO EXPERIENCE: " << std::endl;
    std::cout << "For The Toon Shader: Press T" << std::endl;
    std::cout << "For The Blinn Phong Spotlight (Normals) Shader: Press B" << std::endl;
    std::cout << "For The Phong Direction Lighting Shader: Press P" << std::endl;
    std::cin >> shaderTypeInput;

    //Will wait for user input to decide what shader they wish to experience
    if (shaderTypeInput == "T" || shaderTypeInput == "t")
    {
        switchShader_to_bpToonShader = true;
    }
    else if (shaderTypeInput == "B" || shaderTypeInput == "b")
    {
        switchShader_to_blinnPhongSpotlight = true;
    }
    else if (shaderTypeInput == "P" || shaderTypeInput == "p")
    {
        switchShader_to_phongDirectional = true;
    }
    else 
    {
        std::cout << "WRONG INPUT! " << std::endl;
        exit(EXIT_SUCCESS);
    }

    //Run entire application when user has selected shader e.g. load window, shaders, uniform data, etc.
    SceneRunner runner("A Cabin In The Woods...");

    std::unique_ptr<Scene> scene;

    scene = std::unique_ptr<Scene>(new SceneBasic_Uniform());

    return runner.run(*scene);
}




void SceneBasic_Uniform::initScene()
{
    // Load shader file, link it and activate the shader
    compile();

    //Enable Depth for 3D Rendering
    glEnable(GL_DEPTH_TEST);

    projection = mat4(1.0f);
    angle = 0.0;

    //Load Textures (Diffuse/Normal or Diffuse)
    refreshShader();

    //Initialise the ImGUI for the Render Method
    ImGuiSetup();

    //Start The Music
    toggleMusic();
}




void SceneBasic_Uniform::compile()
{
    // Load, Link, and activate the Phong Shader (Directional Lighting) if user chooses "P"
    if (switchShader_to_phongDirectional == true)
    {
        try {
            shader.compileShader("shader/phongShader.vert");
            shader.compileShader("shader/phongShader.frag");
            shader.link();
            shader.use();
        }
        catch (GLSLProgramException& e) {
            cerr << e.what() << endl;
            exit(EXIT_FAILURE);
        }
    }



    // Load, Link, and activate the Blinn Phonn Normal Shader (Spotlight Lighting) if user chooses "B"
    if (switchShader_to_blinnPhongSpotlight == true)
    {
        try {
            shader.compileShader("shader/blinnPhongShader.vert");
            shader.compileShader("shader/blinnPhongShader.frag");
            shader.link();
            shader.use();
        }
        catch (GLSLProgramException& e) {
            cerr << e.what() << endl;
            exit(EXIT_FAILURE);
        }
    }



    // Load, Link, and activate the Toon Shader (Directional Lighting) if user chooses "T"
    if (switchShader_to_bpToonShader == true)
    {
        try {
            shader.compileShader("shader/bpToonShader.vert");
            shader.compileShader("shader/bpToonShader.frag");
            shader.link();
            shader.use();
        }
        catch (GLSLProgramException& e) {
            cerr << e.what() << endl;
            exit(EXIT_FAILURE);
        }
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
        angle += 0.8f * deltaT;
        if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
    }

}




void SceneBasic_Uniform::render()
{
    #pragma region Setup lighting positions/directions for the Light/Material Uniform Structure Values
    //Clear the current colour & depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Animate the direct lighting position each loop
    vec4 directLight = vec4(15.0f * cos(angle), 15.0f, 15.0f * sin(angle), 1.0f);
    shader.setUniform("Light.Position", view * directLight);


    //Animate the spotlight lighting position each loop
    vec4 lightPos = vec4(15.0f * cos(angle), 15.0f, 15.0f * sin(angle), 1.0f);
    shader.setUniform("Spot.Position", vec3(view * lightPos));


    //Set the Direction for the spot light
    mat3 normalMatrix = mat3(vec3(view[0]), vec3(view[1]), vec3(view[2]));
    shader.setUniform("Spot.Direction", normalMatrix * vec3(-lightPos));
    #pragma endregion



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
    if (switchShader_to_blinnPhongSpotlight == true) 
    {
        // Alter the Poisition/Rotation/Size of the Ogre Mesh
        model = mat4(1.0f);
        model = glm::translate(model, vec3(3.0f, 4.0f, 5.0f));
        model = glm::rotate(model, glm::radians(70.0f), vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, vec3(2.4f, 2.4f, 2.4f));

        //Set Matrix Values based on data above, load model
        setMatrices();
        ogre->render();
    }
    else
    {
    #pragma region House Model Settings

    // Alter the Poisition/Rotation/Size of the House Mesh
    model = mat4(1.0f);
    model = glm::translate(model, vec3(2.0f, 5.1f, -1.5f));
    model = glm::rotate(model, glm::radians(85.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.4f, 0.4f, 0.4f));

    //Load Matrices Settings & Render House Mesh
    setMatrices();
    houseMesh->render();
    #pragma endregion


    #pragma region Island Model Settings
    // Alter the Poisition/Rotation/Size of the Island Mesh
    model = mat4(1.0f);
    model = glm::translate(model, vec3(-3.0f, 0.0f, -3.0f));
    model = glm::rotate(model, glm::radians(120.0f), vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(1.0f, 1.0f, 1.0f));


    //Load Matrices Settings & Render Island Mesh
    setMatrices();
    islandMesh->render();
    #pragma endregion


    #pragma region Tree Models Settings

    // Alter the Poisition/Rotation/Size of the Tree Mesh
    model = mat4(1.0f);
    model = glm::translate(model, vec3(-12.0f, 8.5f, -2.5f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));

    //Load Matrices Settings & Render Tree Mesh
    setMatrices();
    treeMesh->render();


    //Declare Model
    model = mat4(1.0f);
    model = glm::translate(model, vec3(-2.0f, 4.8f, 0.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    setMatrices();
    treeMesh->render();




    model = mat4(1.0f);
    model = glm::translate(model, vec3(1.0f, 5.0f, -2.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    setMatrices();
    treeMesh->render();


    model = mat4(1.0f);
    model = glm::translate(model, vec3(3.0f, 4.5f, 8.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.5f, 0.5f, 0.5f));
    setMatrices();
    treeMesh->render();


    model = mat4(1.0f);
    model = glm::translate(model, vec3(4.0f, 4.5f, 4.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.2f, 0.2f, 0.2f));
    setMatrices();
    treeMesh->render();


    model = mat4(1.0f);
    model = glm::translate(model, vec3(6.0f, 4.5f, 6.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.3f, 0.3f, 0.3f));
    setMatrices();
    treeMesh->render();


    model = mat4(1.0f);
    model = glm::translate(model, vec3(2.0f, 4.2f, 6.0f));
    model = glm::rotate(model, glm::radians(0.0f), vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, vec3(0.3f, 0.3f, 0.3f));
    setMatrices();
    treeMesh->render();
    #pragma endregion
    }
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

    //Setup the direction of the camera - for the bling phong normal texture shader 
    mat4 directionalView = glm::lookAt(vec3(4.0f, 4.0f, 6.5f), vec3(0.0f, 0.75f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
    view = glm::lookAt(vec3(5.0f, 5.0f, 7.5f), vec3(0.0f, 0.75f, 0.0f), vec3(0.0f, 1.0f, 0.0f));

    projection = mat4(1.0f);
    angle = 0.0;

    //Set the direct light position
    shader.setUniform("Light.Position", directionalView * glm::vec4(1.0f, 25.0f, 0.0f, 0.0f));
   
    #pragma region Assign the textures - depending on the shader chosen - Normal Textures or Textures
    // Alters the specular settings, ambient light,
    if (switchShader_to_blinnPhongSpotlight == true)
    {
        // Load diffuse texture
        GLuint diffuseTexture = Texture::loadTexture("media/texture/ogre_diffuse.png");

        // Load normal map texture
        GLuint normalMapTexture = Texture::loadTexture("media/texture/ogre_normalmap.png");

        // Load Diffuse & Normal texture and bind it to the active meshes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normalMapTexture);
    }
    else
    {
        // Load texture and bind it to the active meshes
        GLuint texID = Texture::loadTexture("media/nice69-32x.png");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

    }
    #pragma endregion

}




void SceneBasic_Uniform::toggleMusic()
{
    if (toggleCurrentMusic == true) 
    {
        // Loop background music in game and set the volume
        music = backgoundMusic->play2D("media/audio/Dreamer-by-Hazy.flac", true, false, true);
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
