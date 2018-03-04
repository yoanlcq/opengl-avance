#version 330

uniform vec2 uScale;
uniform vec2 uPosition;

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoords;

out vec2 vTexCoords;

void main() {
    vTexCoords = aTexCoords;
    gl_Position = vec4(uPosition + uScale * aPosition, 0, 1);
}
