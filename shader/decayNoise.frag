#version 460


uniform struct LightInfo 
{
    vec4 Position;  // Eye coordinate Location
    vec3 Intensity; // Light Intensity
} Light;



uniform struct MaterialInfo 
{
  vec3 Ka;          // Ambient Reflectivity
  vec3 Kd;          // Diffuse Reflectivity
  vec3 Ks;          // Specular Reflectivity
  float Shininess;  // Specular Shininess
} Material;

uniform sampler2D NoiseTex;

in vec4 Position;
in vec3 Normal;
in vec2 TexCoord;

uniform float LowThreshold;     // Store Low Threshold Value For Discarding
uniform float HighThreshold;    // Store High Value For Discarding

layout (location = 0) out vec4 FragColor;

vec3 phongModel() 
{
    vec3 n = Normal;                                        // Calcuate the normalized normal
    vec3 s = normalize(Light.Position.xyz - Position.xyz);  // Calculate Specular lighting vector in the view space 
    vec3 v = normalize(-Position.xyz);                      // Calculate Direction    
    vec3 r = reflect(-s, n);                                // Calcuate the reflection

    float sDotN = max( dot(s,n), 0.0);                      // Calculate the conversion for our Diffuse Lighting

    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;   // Calcuate Diffuse Lighting
    vec3 spec = vec3(0.0);

    // Calculate clamped dot product of our reflection and direction - retrieve cosine angle between them
    if(sDotN > 0. ) spec = Light.Intensity * Material.Ks * pow( max( dot(r,v), 0.0 ), Material.Shininess );
    return diffuse + spec;
}

void main()
{
    vec4 noise = texture(NoiseTex, TexCoord);   // Get the noise value at TexCoord

    if(noise.a < LowThreshold) discard;         // If the noise value is less than the threshhold discard noise fragments
    if(noise.a > HighThreshold) discard;        // If the noise value is greater than the threshhold discard noise fragments 

    // Color the fragment using the shading model 
    vec3 color = phongModel();              
    FragColor = vec4(color , 1.0);
}