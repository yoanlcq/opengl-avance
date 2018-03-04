#version 330

uniform mat4 uModelViewProjMatrix;

layout(location = 0) in vec3 aPosition;

out vec3 vPosition;

void main() {
    vPosition = aPosition;
    gl_Position = uModelViewProjMatrix * vec4(aPosition, 1.0f);
}
