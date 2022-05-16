#version 460

const float PI = 3.14159265359;

layout (location = 0) in vec3 VertexPosition;   // Vertex Position Coordinates
layout (location = 1) in vec3 VertexVelocity;   // Vertex Velocity
layout (location = 2) in float VertexAge;       // Vertex Age


uniform int Pass;


// Output To Transform The Feedback Buffers (First Pass)
out vec3 Position;  //Outputs Current Position
out vec3 Velocity;  //Outputs Veloctiy
out float Age;      //Outputs Age


// Output To The Fragment Shader (Second Pass)
out float Transp;
out vec2 TexCoord;


// Imports The Values Of 
// The Particle System
uniform float Time;                      // Particle Simulation Time
uniform float DeltaT;                    // Elapsed Time Between Frames
uniform vec3 Accel;                      // Particle Acceleration
uniform float ParticleLifetime;          // Particle Lifespan
uniform float MinParticleSize = 0.1;     // Minimum Particle Size
uniform float MaxParticleSize = 2.5;     // Maximum Particle Size
uniform mat3 EmitterBasis;               // Emitter Direction
uniform vec3 Emitter;                    // Emitter Position


uniform mat4 MV;
uniform mat4 Proj;


uniform sampler1D RandomTex;


// Offsets To The Position In Camera Coords For Each Vertex Of The Particle's Quad
const vec3 offsets[] = vec3[](vec3(-0.5,-0.5,0), vec3(0.5,-0.5,0), vec3(0.5,0.5,0), vec3(-0.5,-0.5,0), vec3(0.5,0.5,0), vec3(-0.5,0.5,0) );

// Texture Coords For Each Vertex Of The Particle's Quad
const vec2 texCoords[] = vec2[](vec2(0,0), vec2(1,0), vec2(1,1), vec2(0,0), vec2(1,1), vec2(0,1));


vec3 randomInitialVelocity() 
{
    float theta = mix(0.0, PI / 1.5, texelFetch(RandomTex, 3 * gl_VertexID, 0).r);
    float phi = mix(0.0, 2.0 * PI, texelFetch(RandomTex, 3 * gl_VertexID + 1, 0).r);
    float velocity = mix(0.1, 0.2, texelFetch(RandomTex, 3 * gl_VertexID + 2, 0).r);

    vec3 v = vec3(sin(theta) * cos(phi), cos(theta), sin(theta) * sin(phi));
    return normalize(EmitterBasis * v) * velocity;
}


void update() 
{
    if(VertexAge < 0 || VertexAge > ParticleLifetime) 
    {
        // The particle is past it's lifetime, recycle.
        Position = Emitter;
        Velocity = randomInitialVelocity();

        if(VertexAge < 0 ) Age = VertexAge + DeltaT;
        else Age = (VertexAge - ParticleLifetime) + DeltaT;
    }
    else 
    {
        // The particle is alive, update.
        Position = VertexPosition + VertexVelocity * DeltaT;
        Velocity = VertexVelocity + Accel * DeltaT;
        Age = VertexAge + DeltaT;
    }
}


void render() 
{
    Transp = 0.0;
    vec3 posCam = vec3(0.0);

    if( VertexAge >= 0.0 ) 
    {
        float agePct = VertexAge / ParticleLifetime;
        Transp = clamp(1.0 - agePct, 0, 1);
        posCam = (MV * vec4(VertexPosition,1)).xyz + offsets[gl_VertexID] * mix(MinParticleSize, MaxParticleSize, agePct);
    }

    TexCoord = texCoords[gl_VertexID];

    gl_Position = Proj * vec4(posCam,1);
}


void main() {
    if(Pass == 1)
        update();
    else
        render();
}