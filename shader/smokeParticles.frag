#version 460


// Out To Transform Feedback Buffers (first pass)
in vec3 Position;


in float Transp;    // Transposition
in vec2 TexCoord;   // Texture Coordinates


uniform sampler2D ParticleTex;              // Particle texture
layout (location = 0) out vec4 FragColor;   // Output Fragment Colour


void main()
{
    // Setting The Frag Colour To The Texture's Colour
    FragColor = texture(ParticleTex, TexCoord);
    FragColor.a *= Transp;
}