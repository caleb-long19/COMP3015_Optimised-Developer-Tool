#version 460


layout(binding=1) uniform samplerCube SkyBoxTex;


in vec3 Vec;    // Retrieve vertex position from skybox.vert


layout( location = 0 ) out vec4 FragColor;


void main() 
{
    // Getting the colour of our skybox by using the samplercube & Vec values
    vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;

    // Raises to power, used for Gamma Correction
    texColor = pow( texColor, vec3(1.0/2.2));

    // Pass the result into FragColour
    FragColor = vec4(texColor,1);
}