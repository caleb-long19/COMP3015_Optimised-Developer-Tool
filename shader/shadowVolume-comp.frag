#version 460

in vec3 Position; 
in vec3 Normal;

uniform sampler2D DiffSpecTex;				// Diffused/Specular Texture
layout(location = 0) out vec4 FragColor;	// Output Fragment Colour

void main() 
{
  vec4 diffSpec = texelFetch(DiffSpecTex, ivec2(gl_FragCoord), 0);
  FragColor = vec4(diffSpec.xyz, 1);
}