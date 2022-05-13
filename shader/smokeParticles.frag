#version 460

uniform sampler2D ParticleTex;

// Out to transform feedback buffers (first pass)
in vec3 Position;

in float Transp;
in vec2 TexCoord;

layout ( location = 0 ) out vec4 FragColor;

void main()
{
    FragColor = texture(ParticleTex, TexCoord);
    FragColor.a *= Transp;
}