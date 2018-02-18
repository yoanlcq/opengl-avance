#version 430

layout(local_size_x = 1024) in;
layout(std430, binding = 0) buffer posB { vec4 pos[]; };
layout(std430, binding = 1) buffer velB { vec4 vel[]; };

uniform uint uParticleCount;
uniform float uDeltaTime;
uniform float uVelMultiplier;

void main() {
    uint i = uint(gl_GlobalInvocationID.x);
    if(i >= uParticleCount)
        return;
    pos[i].xyz += uDeltaTime * uVelMultiplier * vel[i].xyz;
}

