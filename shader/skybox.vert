#version 460


layout (location = 0) in vec3 VertexPosition; // Vertex Position Coordinates


out vec3 Vec;

uniform mat4 MVP;   // Model, View, Projection


void main() 
{
    Vec = VertexPosition;                           // Storing/Setting The Vertex Postion Into Vec
    gl_Position = MVP * vec4(VertexPosition,1.0);   // Store The Current Vertex Position
}