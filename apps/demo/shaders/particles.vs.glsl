#version 330

uniform mat4 uModelViewProjMatrix;

layout(location = 0) in vec3 aPosition;

void main() {
    vec4 pos = uModelViewProjMatrix * vec4(aPosition, 1);
    gl_Position = pos;
    // gl_PointSize = 400.f / pos.z;
    gl_PointSize = 4.f;
}
