#version 330

out vec4 fColor;

void main() {
    vec2 fromCenter = gl_PointCoord - vec2(0.5f);
    float d = length(fromCenter);
    float alpha = 2.f * (0.5f - d);
    vec3 hi = vec3(1.0f, 0.8f, 0.2f);
    vec3 lo = vec3(1.0f, 0.0f, 0.0f);
    vec3 color = mix(lo, hi, alpha);
    fColor = vec4(color, alpha);
}
