#version 330

layout(location = 0) in vec4 aPosition;

out float vLife;

uniform mat4 uModelViewProjMatrix;
uniform float uPointSize;
uniform float uZScale;
uniform float uZInfluence;

void main() {
    vec4 pos = uModelViewProjMatrix * vec4(aPosition.xyz, 1);
    vLife = aPosition.w;
    gl_Position = pos;
    gl_PointSize = mix(uPointSize, uPointSize * uZScale / pos.z, uZInfluence);
}
