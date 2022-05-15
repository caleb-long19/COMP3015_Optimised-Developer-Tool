#version 460

// Retrieved from SceneBasic_Uniform.cpp

layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexNormal;     // Vertex Normals
layout (location = 2) in vec2 VertexTexCoord;   // Vertex Texture Coordinates



out vec3 Position;  // Outputs Current Position
out vec3 Normal;    // Outputs Normal
out vec2 TexCoord;  // Outputs Texture Coordinates


uniform mat4 ModelViewMatrix;   // Model View Matrix
uniform mat3 NormalMatrix;      // Normal Matrix
uniform mat4 ProjMatrix;        // Projection matrix


void main()
{
    TexCoord = VertexTexCoord;                                              // Set/Store The Texture Coordinates
    Normal = normalize(NormalMatrix * VertexNormal);                        // Assign Normal to Normal Vertex Data
    Position = vec3(ModelViewMatrix * vec4(VertexPosition,1.0));            // Calculate The Current Position
    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition,1.0);  // Store The Current Vertex Position
}