#version 460

in vec3 Position;    // Retrieve vertex position from render.vert
in vec3 Normal;      // Retrieve normal from render.vert
in vec2 TexCoord;    // Retrieve vertex position from render.vert

uniform vec4 LightPosition;     // Position of the light in the scene
uniform vec3 LightIntensity;


uniform sampler2D Tex;      // Geometry Texture
uniform sampler2D NoiseTex; // Noise Texture
uniform int shaderType;     // Shader Type Value


uniform vec3 Kd;            // Diffuse Reflectivity
uniform vec3 Ka;            // Ambient Reflectivity
uniform vec3 Ks;            // Specular Reflectivity
uniform float Shininess;    // Specular Shininess Factor


layout(location = 0) out vec4 Ambient;      // Vertex Ambient Data
layout(location = 1) out vec4 DiffSpec;     // Diffuse/Specular Data


uniform float LowThreshold;     // Store Low Threshold Value For Discarding
uniform float HighThreshold;    // Store High Value For Discarding

const int levels = 4;
const float scaleFactor = 1.0 / levels;


// Phong Shading Model
void phongShade()
{
    vec3 specular = normalize(vec3(LightPosition) - Position); // Calculate Specular lighting vector in the view space 

    vec3 v = normalize(vec3(-Position));    // Calculate Direction
    vec3 r = reflect(-specular, Normal);    // Calcuate the reflection
    vec4 texColor = texture(Tex, TexCoord); // get texColor value at texture coordinates

    // Calculate The Ambient Lighting
    Ambient = vec4(texColor.rgb * LightIntensity * Ka, 1.0);

    // Calculate The Diffuse/Specular Lighting
    DiffSpec = vec4(texColor.rgb * LightIntensity * (Kd * max( dot(specular, Normal), 0.0) + Ks * pow(max(dot(r,v), 0.0), Shininess)) ,1.0 );
}


void toonShade()
{
    vec3 texColor = texture(Tex, TexCoord).rgb;
	vec3 specular = normalize(LightPosition.xyz - Position.xyz);

    // Calculate The Ambient Lighting
	Ambient = vec4(texColor.rgb * LightIntensity * Ka, 1.0);

	float cosine = dot(specular, Normal);

    // Calculate The Diffuse/Specular Lighting
	DiffSpec = vec4(texColor * Kd * ceil(cosine * levels) * scaleFactor, 1);
}

void main() 
{
    // Render Phong Shading
    if(shaderType == 0)
    {
        phongShade();
    }
    else if(shaderType == 1)
    {
        toonShade();
    }
    else if (shaderType == 2)
    {
        vec4 noise = texture(NoiseTex, TexCoord);   // Get the noise value at TexCoord

        if(noise.a < LowThreshold) discard;         // If the noise value is less than the threshhold discard noise fragments
        if(noise.a > HighThreshold) discard;        // If the noise value is greater than the threshhold discard noise fragments 

        phongShade();
    }

}

