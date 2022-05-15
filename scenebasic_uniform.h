#ifndef SCENEBASIC_UNIFORM_H
#define SCENEBASIC_UNIFORM_H

#include "helper/scene.h"
#include "helper/objmesh.h"
#include "helper/skybox.h"
#include "helper/frustum.h"

// Sound Library - Used for background forestAmbience
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
    GLSLProgram renderShader, volumeShader, compShader, skyShader, smokeShader;
    GLuint colorDepthFBO, fsQuad, quad;
    GLuint modelTex, dsModelTex;

    // Position and direction of emitter.
    // Int Arrays for Position, Veloctiy and Age Buffers
    glm::vec3 emitterPos, emitterDir;
    GLuint posBuf[2], velBuf[2], age[2];
    GLuint particleArray[2];
    GLuint feedback[2];
    GLuint drawBuf;

    int nParticles;                         // Number of Particles
    int shadowMapWidth, shadowMapHeight;    // Shadow Volume Data
    int shaderSwitch = 0;


    glm::mat4 lightPV, shadowBias;
    glm::vec4 lightPos;                                             // Position of the Light


    float camAngle, tPrev, time, deltaT, rotSpeed;                  // Used to reclculate the camAngle of the lighting, time it takes, and how fast the animation plays
    float vehicleAngle, vehicleSpeed;                               // Used for calculating the camAngle the moves & how fast the vehicle animation plays
    float particleLifetime;                                         // Length of time in which particles stay alive
    float lightPosX = 0.5f, lightPosY = 1.5f, lightPosZ = 4.5f;     // Values to alter the position of the global light (Used in the GUI)


#pragma region MyRegion

    // Setup ObjMesh Variables 
    // Used to load the models
    std::unique_ptr<ObjMesh> townMesh;          // Street mesh
    std::unique_ptr<ObjMesh> whiteHouse;        // White House mesh
    std::unique_ptr<ObjMesh> yellowHouse;       // Yellow House mesh
    std::unique_ptr<ObjMesh> blueHouse;         // Blue House mesh
    std::unique_ptr<ObjMesh> redHouse;          // Red House mesh

    std::unique_ptr<ObjMesh> fenceMesh;         // Fence mesh
    std::unique_ptr<ObjMesh> lamp_postMesh;     // Lamp Post mesh
    std::unique_ptr<ObjMesh> treeMesh;          // Tree mesh

    std::unique_ptr<ObjMesh> yellowCarMesh;     // Red Car mesh
    std::unique_ptr<ObjMesh> redCarMesh;        // Yellow Car mesh
    std::unique_ptr<ObjMesh> planeDecay;        // Plane mesh

#pragma endregion


    SkyBox sky;
    Frustum lightFrustum;


#pragma region Methods For SceneBasic_Uniform.cpp

    // Method to contain the matrices data of the different shaders
    void setMatrices(GLSLProgram &);
    void setSkyboxMatrices(GLSLProgram &);
    void setParticleMatrices(GLSLProgram &);


    // Compile, setup buffer data, and draw the scene methods
    void compile();
    void setupFBO();
    void drawScene(GLSLProgram&, bool);
    void initBuffers();
    void pass1();
    void pass2();
    void pass3();
    void updateLight();


    // Setup methods for the different techniques
    void setupParticles();
    void setupSkybox();
    void setupShadowVolumes();

#pragma endregion

public:
    SceneBasic_Uniform();

    void initScene();
    void toggleAmbience();
    void ImGuiSetup();

    void update( float t );
    void render();
    void resize(int, int);
};

#endif // SCENEBASIC_UNIFORM_H
