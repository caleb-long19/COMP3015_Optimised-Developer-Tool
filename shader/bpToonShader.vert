#version 460

//Information is retrived from the scenebasic_uniform.cpp

// Vertex Position Coordinates
layout (location = 0) in vec3 VertexPosition;

// Vertex Normals
layout (location = 1) in vec3 VertexNormal;

// Vertex Texture Coordinates
layout (location = 2) in vec2 VertexTexCoord;


//Outputs Current Position
out vec3 Position;

//Outputs Normal
out vec3 Normal;

//Outputs Texture Coordinates
out vec2 TexCoord;


// Imports The Model View Matrix - Translation, Rotation, Scale
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;



void main() {

    //Assign Texture Coordinates to the Vertex Texture Data
    TexCoord = VertexTexCoord;

    //Assign Normal to Normal vertex Data
    Normal = normalize( NormalMatrix * VertexNormal);

    //Calculate the current position
    Position = (ModelViewMatrix * vec4(VertexPosition,1.0)).xyz;

     //Store current vertex position
    gl_Position = MVP * vec4(VertexPosition,1.0);
}