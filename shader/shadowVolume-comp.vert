#version 460

layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexNormal;     // Vertex Normals


out vec3 Position;     // Outputs Current Position
out vec3 Normal;       // Outputs Normal


// Imports The Model View Matrix - Translation, Rotation, Scale Of A Model
uniform mat4 ModelViewMatrix;   // Model View Matrix
uniform mat3 NormalMatrix;      // Normal Matrix
uniform mat4 ProjMatrix;        // Projection Matrix


void main()
{
    Normal = normalize(NormalMatrix * VertexNormal);                        // Assign Normal to Normal Vertex Data
    Position = vec3(ModelViewMatrix * vec4(VertexPosition,1.0));            // Calculate The Current Position
    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition,1.0);  // Store The Current Vertex Position
}