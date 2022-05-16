#version 460


in vec3 Position;    // Retrieve vertex position from comp.vert
in vec3 Normal;      // Retrieve normal from comp.vert


uniform sampler2D DiffSpecTex;				// Diffused/Specular Texture
layout(location = 0) out vec4 FragColor;	// Output Fragment Colour


void main() 
{
  // Calculate the diffuse + specular vector
  vec4 diffSpec = texelFetch(DiffSpecTex, ivec2(gl_FragCoord), 0);

  // Set fragment colour to diffuse + specular vector
  FragColor = vec4(diffSpec.xyz, 1);
}