#version 330

uniform sampler2D uTexture;
uniform vec2 uTexCoordsOffset;
uniform vec2 uTexCoordsSize;
uniform float uAlphaMultiplier;

in vec2 vTexCoords;

out vec4 fColor;

void main() {
    vec4 color = texture(uTexture, uTexCoordsOffset + uTexCoordsSize * vTexCoords);
    color.a *= uAlphaMultiplier;
    fColor = color;
}
