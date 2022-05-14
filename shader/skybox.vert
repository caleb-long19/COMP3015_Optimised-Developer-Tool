#version 460


layout (location = 0) in vec3 VertexPosition; // Vertex Position Coordinates


// Output to skybox.frag
out vec3 Vec;

// Model, View, Projection
uniform mat4 MVP;


void main() 
{
    // Storing/Setting the vertex postion into Vec
    // Used in skybox.frag
    Vec = VertexPosition;
    gl_Position = MVP * vec4(VertexPosition,1.0);
}