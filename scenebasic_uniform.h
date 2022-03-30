#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/objmesh.h"
#include "helper/skybox.h"

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

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram shader;

    //Imported Meshes
    std::unique_ptr<ObjMesh> houseMesh; //House mesh
    std::unique_ptr<ObjMesh> islandMesh; //Island mesh
    std::unique_ptr<ObjMesh> treeMesh; //Tree mesh

    std::unique_ptr<ObjMesh> ogre; //Ogre mesh

    //Angle (used for animating objects e.g. lighting position), rotation speeds
    float angle, tPrev, rotSpeed;

    //Float values for Uniform Data
    float specularShininessValue = 20.0f, spotExponentValue = 20.0f, spotCutoffValue = 30.0f, fogMinDistanceValue = 0.0f, fogMaxDistanceValue = 11.5f;

    void setMatrices();
    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void refreshShader();
    void toggleMusic();
    void ImGuiSetup();
    void sceneShaderSetup();
    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
