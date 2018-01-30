#version 330

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;
uniform float uKaSamplerFactor;
uniform float uKdSamplerFactor;
uniform float uKsSamplerFactor;
uniform float uShininessSamplerFactor;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec3 vWorldSpaceNormal;
in vec2 vTexCoords;

out vec4 fColor;

void main() {
    vec4 texel = texture(uKdSampler, vTexCoords);
    float distToPointLight = length(uPointLightPosition - vViewSpacePosition);
    vec3 dirToPointLight = (uPointLightPosition - vViewSpacePosition) / distToPointLight;
    vec3 color = texel.xyz * uKd * (uDirectionalLightIntensity * max(0.0, dot(vViewSpaceNormal, uDirectionalLightDir)) + uPointLightIntensity * max(0.0, dot(vViewSpaceNormal, dirToPointLight)) / (distToPointLight * distToPointLight));
    fColor = vec4(color, texel.a);
}
