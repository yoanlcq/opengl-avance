#version 330

uniform vec4 uInnerColor;
uniform vec4 uOuterColor;

in float vLife;

out vec4 fColor;

void main() {
    vec2 fromCenter = gl_PointCoord - vec2(0.5f);
    float d = length(fromCenter);
    float alpha = clamp(2.f * (0.5f - d), 0.f, 1.f);
    vec4 color = mix(uOuterColor, uInnerColor, alpha);
    color *= clamp(vLife, 0, 1);
    fColor = color;
}
