#version 460

//Information is retrived from the scenebasic_uniform.cpp

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;


layout(binding=0) uniform sampler2D Tex1;


uniform struct LightInfo {
    vec4 Position; // Eye coordinate Location

    vec3 L; // Diffuse & Specular Intensity

    vec3 La; //Ambience Intensity
} Light;



uniform struct MaterialInfo {
  vec3 Ka; // Ambient Reflectivity

  vec3 Kd; // Diffuse Reflectivity
} Material;



uniform struct FogInfo {
  float MaxDist; //Max distance of fog

  float MinDist; //Min distance of fog

  vec3 Color; //Fog Colour
} Fog;



const int levels = 4;
const float scaleFactor = 1.0 / levels;


layout( location = 0 ) out vec4 FragColor;


vec3 bpToonShader( ) {

    //Calculate the texture we want to sample with the coordinates of the texture
    vec3 texColor = texture(Tex1, TexCoord).rgb;

    //Calcuate the normalized normal
    vec3 normal = normalize(Normal);

    //Calculate Specular lighting vector in the view space 
    vec3 specular = normalize(Light.Position.xyz - Position);

    //Calculate the ambient light
    vec3 ambient = Light.La * texColor;

    //Calculate the conversion for our Diffuse Lighting
    float sDotN = max(dot(specular, normal), 0.0);

    //Calcuate Diffuse Lighting
    vec3 diffuse = texColor * floor(sDotN * levels) * scaleFactor;

    //Calculate the Toon Shader - Ambient + Diffuse; - Return Colour
    return ambient + Light.L * diffuse;
}

void main() {

   //Calculate the absolute value of Position.z and store in distance
   float fDist = abs(Position.z);
   float fFactor = (Fog.MaxDist - fDist)/(Fog.MaxDist - Fog.MinDist); 

   //constrain the fog distance between two values (Min, Max)
   fFactor = clamp(fFactor, 0.0, 1.0);

   //Store the colour output of our shader
   vec3 shadeColor = bpToonShader();

   //Linear Interpolation - find the weight between all 3 values
   vec3 color = mix(Fog.Color, shadeColor, fFactor);

   //Pass the result into FragColour
   FragColor = vec4(color, 1.0);
}