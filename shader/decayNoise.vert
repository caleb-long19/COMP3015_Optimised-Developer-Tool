#version 460

layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexNormal;     // Vertex Normals
layout (location = 2) in vec2 VertexTexCoord;   // Vertex Texture Coordinates


out vec4 Position;  // Outputs Current Position
out vec3 Normal;    // Outputs Normal
out vec2 TexCoord;  // Outputs Texture Coordinates


uniform mat4 ModelViewMatrix;   // Model View Matrix
uniform mat3 NormalMatrix;      // Normal Matrix
uniform mat4 MVP;               // Model View Projection


void main()
{
    Normal = NormalMatrix * VertexNormal;                   // Assign Normal to Normal Vertex Data
    Position = ModelViewMatrix * vec4(VertexPosition,1.0);  // Calculate The Current Position
    TexCoord = VertexTexCoord;                              // Set/Store The Texture Coordinates
    gl_Position = MVP * vec4(VertexPosition, 1.0);          // Store The Current Vertex Position
}