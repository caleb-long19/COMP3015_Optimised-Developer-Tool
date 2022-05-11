#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/objmesh.h"
#include "helper/skybox.h"
#include "helper/frustum.h"

// Sound Library - Used for background music
#include <irrklang/irrklang.h>
using namespace irrklang;


// ImGUI Library - Used for the GUI elements on screen
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include "helper/glslprogram.h"
#include "helper/texture.h"

#include <glm/glm.hpp>
#include "helper/plane.h"




class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram renderShader, volumeShader, compShader;
    GLuint colorDepthFBO, fsQuad;
    GLuint spotTex, brickTex;



    //Imported Meshes
    Plane plane;
    std::unique_ptr<ObjMesh> spot; //House mesh
    std::unique_ptr<ObjMesh> streetMesh; //House mesh
    std::unique_ptr<ObjMesh> yellowCarMesh; //Red Car mesh
    std::unique_ptr<ObjMesh> redCarMesh; //Yellow Car mesh
    std::unique_ptr<ObjMesh> lamp_postMesh; //Tree mesh
    std::unique_ptr<ObjMesh> treeMesh; //Tree mesh




    //Shadow Map
    int shadowMapWidth, shadowMapHeight;
    glm::mat4 lightPV, shadowBias;
    Frustum lightFrustum;



    //Angle (used for animating objects e.g. lighting position), rotation speeds
    glm::vec4 lightPos;
    float angle, tPrev, rotSpeed;



    #pragma region MyRegion

    //Float values for Uniform Data
    float specularShininessValue = 20.0f, spotExponentValue = 20.0f, spotCutoffValue = 30.0f, fogMinDistanceValue = 0.0f, fogMaxDistanceValue = 11.5f;

    #pragma endregion



    void setMatrices(GLSLProgram &);
    void compile();
    void setupFBO();
    void drawScene(GLSLProgram&, bool);


    void pass1();
    void pass2();
    void pass3();
    void updateLight();

public:
    SceneBasic_Uniform();

    void initScene();
    void toggleMusic();
    void ImGuiSetup();

    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
