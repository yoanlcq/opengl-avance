#version 330

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec3 vWorldSpaceNormal;
out vec2 vTexCoords;

void main() {
    vec4 aPosition4 = vec4(aPosition, 1);
    vec4 aNormal4 = vec4(aNormal, 0);
    vViewSpacePosition = vec3(uModelViewMatrix * aPosition4);
    vViewSpaceNormal = vec3(uNormalMatrix * aNormal4);
    vWorldSpaceNormal = aNormal;
    vTexCoords = aTexCoords;
    gl_Position = uModelViewProjMatrix * aPosition4;
}
