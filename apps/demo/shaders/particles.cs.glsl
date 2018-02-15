#version 430

layout(local_size_x = 1024) in;
layout(std430, binding = 0) buffer posB { vec3 pos[]; };
layout(std430, binding = 1) buffer velB { vec3 vel[]; };

uniform uint uParticleCount;
uniform float uDeltaTime;

void main() {
    uint i = uint(gl_GlobalInvocationID.x);
    if(i >= uParticleCount)
        return;
    pos[i].xyz += uDeltaTime * vel[i].xyz;
}

