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

  vec3 Ks; // Specular Reflectivity

  float Shininess; // Specular Shininess
} Material;



uniform struct FogInfo {
  float MaxDist; //Max distance of fog

  float MinDist; //Min distance of fog

  vec3 Color; //Fog Colour
} Fog;



layout( location = 0 ) out vec4 FragColor;



vec3 phongModel(vec3 position, vec3 normal) {

  //Calculate the texture we want to sample with the coordinates of the texture
  vec3 texColor = texture(Tex1, TexCoord).rgb;

  //Calculate Ambient Light
  vec3 ambient = Light.La * texColor;

  //Create specular vector
  vec3 specular;

  //Calculate the (Specular) lighting vector in the view space 
  if( Light.Position.w == 0.0)
    specular = normalize(Light.Position.xyz);
  else
    specular = normalize(Light.Position.xyz - position);

  //Store the greatest dot product value (Scalar) of specular & normal
  float sDotN = max(dot(specular,normal), 0.0);

  //Calculate Diffuse Lighting with the texColour and max dot product
  vec3 diffuse = texColor * sDotN;


  //Calculate Specular Lighitng
  vec3 spec = vec3(0.0);
  if(sDotN > 0.0)
  {
    //Calculate Direction
    vec3 v = normalize(-position.xyz); 

    //Calcuate the reflection
    vec3 r = reflect(-specular, normal); 

    //Calculate clamped dot product of our reflection and direction - retrieve cosine angle between them
    spec = Material.Ks * pow(max(dot(r,v), 0.0), Material.Shininess);
  }

  //Calculate the phong model - Ambient + Diffuse + Specular
  return ambient + Light.L * (diffuse + spec);
}



void main()
{

   //Store the absolute value of Position.z and store in distance
   float fDist = abs(Position.z);
   float fFactor = (Fog.MaxDist - fDist)/(Fog.MaxDist - Fog.MinDist); 

    //constrant the fog distance between two values (Min, Max)
   fFactor = clamp(fFactor, 0.0, 1.0);

   //Store the colour output of our shader
   vec3 shadeColor = phongModel(Position, normalize(Normal));

   //Linear Interpolation - find the weight between all 3 values
   vec3 color = mix(Fog.Color, shadeColor, fFactor);

   //Pass the result into FragColour
   FragColor = vec4(color, 1.0);
}