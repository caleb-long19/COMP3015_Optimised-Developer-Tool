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
#include "helper/noisetex.h"
#include "helper/skybox.h"
#include "helper/particleutils.h"


#include <glm/glm.hpp>
#include "helper/plane.h"
#include "helper/grid.h"


class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram renderShader, volumeShader, compShader, skyShader, smokeShader, flatShader;
    GLuint colorDepthFBO, fsQuad, quad;
    GLuint spotTex, brickTex;

    // Position and direction of emitter.
    glm::vec3 emitterPos, emitterDir;
    GLuint posBuf[2], velBuf[2], age[2];
    GLuint particleArray[2];
    GLuint feedback[2];
    GLuint drawBuf;
    
    SkyBox sky;


    //Imported Meshes
    std::unique_ptr<ObjMesh> streetMesh; //Street mesh
    std::unique_ptr<ObjMesh> defaultHouse; //House mesh
    std::unique_ptr<ObjMesh> yellowHouse; //House mesh
    std::unique_ptr<ObjMesh> blueHouse; //House mesh
    std::unique_ptr<ObjMesh> redHouse; //House mesh

    std::unique_ptr<ObjMesh> fenceMesh; //House mesh
    std::unique_ptr<ObjMesh> lamp_postMesh; //Tree mesh
    std::unique_ptr<ObjMesh> treeMesh; //Tree mesh

    std::unique_ptr<ObjMesh> yellowCarMesh; //Red Car mesh
    std::unique_ptr<ObjMesh> redCarMesh; //Yellow Car mesh



    //Shadow Map
    int shadowMapWidth, shadowMapHeight;
    glm::mat4 lightPV, shadowBias;
    Frustum lightFrustum;



    //Angle (used for animating objects e.g. lighting position), rotation speeds
    glm::vec4 lightPos;
    int nParticles;
    float angle, tPrev, time, deltaT, rotSpeed, carAngle, carSpeed, particleLifetime;




    void setMatrices(GLSLProgram &);
    void setSkyboxMatrices(GLSLProgram &);
    void setParticleMatrices(GLSLProgram &);
    void compile();
    void setupFBO();
    void drawScene(GLSLProgram&, bool);
    void initBuffers();

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
