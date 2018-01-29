#version 330

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec3 vWorldSpaceNormal;
in vec2 vTexCoords;

out vec4 fColor;

void main() {
    fColor = vec4(vWorldSpaceNormal, 1);
}
