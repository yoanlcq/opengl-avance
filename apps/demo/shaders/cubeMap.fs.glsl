#version 330

uniform samplerCube uCubeMap;

in vec3 vPosition;

out vec4 fColor;

void main() {
    fColor = texture(uCubeMap, vPosition);
}
