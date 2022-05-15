#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;


uniform vec4 LightPosition;
uniform vec3 LightIntensity;


uniform sampler2D Tex;      // Active Texture


uniform vec3 Kd;            // Diffuse Reflectivity
uniform vec3 Ka;            // Ambient Reflectivity
uniform vec3 Ks;            // Specular Reflectivity
uniform float Shininess;    // Specular Shininess Factor


layout(location = 0) out vec4 Ambient;      // Vertex Ambient Data
layout(location = 1) out vec4 DiffSpec;     // Diffuse/Specular Data


// Phong Shading Model
void shade( )
{
    vec3 specular = normalize(vec3(LightPosition) - Position); // Calculate Specular lighting vector in the view space 

    vec3 v = normalize(vec3(-Position));    // Calculate Direction
    vec3 r = reflect(-specular, Normal);    // Calcuate the reflection
    vec4 texColor = texture(Tex, TexCoord); // Store Texel Value (Colour & Alpha Values)

    // Calculate The Ambient Lighting
    Ambient = vec4(texColor.rgb * LightIntensity * Ka, 1.0);

    // Calculate The Diffuse/Specular Lighting
    DiffSpec = vec4(texColor.rgb * LightIntensity * (Kd * max( dot(specular, Normal), 0.0) + Ks * pow(max(dot(r,v), 0.0), Shininess)) ,1.0 );
}


void main() 
{
    // Render Phong Shading
    shade();
}