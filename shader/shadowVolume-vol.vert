#version 460

layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexNormal;     // Vertex Normals
layout (location = 2) in vec2 VertexTexCoord;   // Vertex Texture Coordinates

out vec3 VPosition;     // Outputs Current Position
out vec3 VNormal;       // Outputs Normal

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjMatrix;

void main()
{ 
    VNormal = NormalMatrix * VertexNormal;                                      // Assign Normal to Normal vertex Data
    VPosition = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;               // Calculate the current position
    gl_Position = ProjMatrix * ModelViewMatrix * vec4(VertexPosition,1.0);      // Store current vertex position
}