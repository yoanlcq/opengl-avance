#version 330

out vec4 fColor;

void main() {
    vec2 fromCenter = gl_PointCoord - vec2(0.5f);
    float d = length(fromCenter);
    float alpha = 2f * (0.5f - d);
    const vec3 baseColor = vec3(1f, 0.9f, 0.2f);
    fColor = vec4(baseColor, alpha);
}
