#version 460


layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexNormal;     // Vertex Normals
layout (location = 2) in vec2 VertexTexCoord;   // Vertex Texture Coordinates


out vec3 VPosition;     // Outputs Vectors/Current Position
out vec3 VNormal;       // Outputs Vectors/Normal


// Uniform Data - Retrieved from SceneBasic_Uniform.cpp
uniform mat4 ModelViewMatrix;   // Model View Matrix
uniform mat3 NormalMatrix;      // Normal Matrix
uniform mat4 ProjMatrix;        // Projection Matrix

void main()
{ 
    VNormal = NormalMatrix * VertexNormal;                                      // Assign Normal to Normal Vertex Data
    VPosition = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;               // Calculate The Current Position
    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition,1.0);      // Store The Current Vertex Position
}