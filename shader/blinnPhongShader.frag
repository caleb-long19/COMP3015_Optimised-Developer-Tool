#version 460

//Information is retrived from the scenebasic_uniform.cpp

in vec3 LightDir;
in vec2 TexCoord;
in vec3 ViewDir;

layout(binding=0) uniform sampler2D ColorTex;
layout(binding=1) uniform sampler2D NormalMapTex;


uniform struct SpotLightInformation {
    vec4 Position; // Eye coordinate Location

    vec3 L; // Diffuse & Specular Intensity

    vec3 La; //Ambience Intensity

    vec3 Direction; //Spoitlight direction (Eye coordinates)
    
    float Exponent; // Angular Attenuation

    float Cutoff; // Angle Cutoff
} Spot;



uniform struct MaterialInformation {
  vec3 Ka; // Ambient Reflectivity

  vec3 Kd; // Diffuse Reflectivity

  vec3 Ks; // Specular Reflectivity

  float Shininess; // Specular Shininess

} Material;



layout( location = 0 ) out vec4 FragColor;


vec3 blinnPhongSpot(vec3 n) {  

  //Diffuse & Specular Vectors
  vec3 diffuse;
  vec3 spec;

  //Calculate the texture we want to sample with the coordinates of the texture
  vec3 texColor = texture(ColorTex, TexCoord).rgb;
  
  //Calculate Ambient Lighting - Acquire Spotlight Ambient Uniform Value
  vec3 ambient = Spot.La * texColor;

  //Calculate Specular Lighting
  vec3 specular = normalize(LightDir);

  //Calculate/Store the dot product (Measures direction of two vectors) of Specular & Spotlight Direction
  float cosAng = dot(-specular, normalize(Spot.Direction));

  //Calculate the trigonometric angle of cosine angle and store inside angle
  float angle = acos(cosAng);

  //Size of Spotlight
  float spotScale = 0.0;

  //Check if our angle is greater >= to 0 and less than the cutoff point of the spotlight
  if(angle >= 0.0 && angle < Spot.Cutoff) {

    //Calculate the cosine angle raised to the power of the Exponent value and store in spotScale
    spotScale = pow(cosAng, Spot.Exponent);

    //Store the greatest dot product value (Scalar) of specular & normal
    float sDotN = max(dot(specular,n), 0.0);

    //Calculate Diffuse Lighting with the texColour and max dot product
    diffuse = texColor * sDotN;

    //Calculate Specular Lighitng
    spec = vec3(0.0);
    if(sDotN > 0.0) 
    {
      //Calculate Direction
      vec3 v = normalize(ViewDir);

      //Calculate halfway direction
      vec3 h = normalize(v + specular);

      //Calculate clamped dot product of our normal and halfway direction - retrieve cosine angle between them
      spec = Material.Ks * pow(max(dot(h,n), 0.0), Material.Shininess);
    }

  }
  //Calculate the blinn phong model - Ambient + Diffuse + Specular (Blinn Phong angles are measured between the normal and halfway vector)
  return ambient + spotScale * Spot.L * (diffuse + spec);
}

void main() 
{
  //Calculate the norm by acquiring the normal texture sample along with the coordinates of the texture
  vec3 norm = texture(NormalMapTex, TexCoord).xyz;

  //Set the normal map range
  norm.xy = 2.0 * norm.xy - 1.0;

  //Pass the result into FragColour
  FragColor = vec4(blinnPhongSpot(norm), 1.0);
}