#version 460

//Information is retrived from the scenebasic_uniform.cpp

// Vertex Position Coordinates
layout (location = 0) in vec3 VertexPosition;

// Vertex Normals
layout (location = 1) in vec3 VertexNormal;

// Vertex Texture Coordinates
layout (location = 2) in vec2 VertexTexCoord;

// Vertex Tangent Coordinates
layout (location = 3) in vec4 VertexTangent;



uniform struct LightInfo {
  // Light position in cam coords.
  vec4 Position;

  //Diffuse & Spectural
  vec3 L;

  //Ambient Intensity
  vec3 La;
} Light ;



//Outputs Lighting Direction
out vec3 LightDir;

//Outputs View Direction
out vec3 ViewDir;

//Outputs Texture Coordinates
out vec2 TexCoord;



// Imports The Model View Matrix - Translation, Rotation, Scale
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;



void main() {

    // Transform normal and tangent to eye space coordinates
    vec3 norm = normalize(NormalMatrix * VertexNormal);
    vec3 tang = normalize(NormalMatrix * vec3(VertexTangent));

    // Compute the binormal - Homogeneous Vertex Tangent Cooridnate - Calculate the cross product of our two vectors and normalize (Calculate unit vector in same dir as the original vector)
    vec3 binormal = normalize(cross(norm, tang)) * VertexTangent.w;

    // Matrix for transformation to tangent space
    mat3 toObjectLocal = mat3(tang.x, binormal.x, norm.x, tang.y, binormal.y, norm.y, tang.z, binormal.z, norm.z);

    // Transform light direction and view direction to tangent space
    vec3 pos = vec3(ModelViewMatrix * vec4(VertexPosition, 1.0));

    //Assign our Lighting Direction
    LightDir = toObjectLocal * (Light.Position.xyz - pos);

    //Assign our View Direction
    ViewDir = toObjectLocal * normalize(-pos);

    //Assign Texture Coordinates to the Vertex Texture Data
    TexCoord = VertexTexCoord;

    //Store current vertex position
    gl_Position = MVP * vec4(VertexPosition, 1.0);

}