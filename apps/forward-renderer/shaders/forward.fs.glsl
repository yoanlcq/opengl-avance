#version 330

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;
uniform float uPointLightRange;
uniform float uPointLightAttenuationFactor;

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

    vec4 KaSampled = texture(uKaSampler, vTexCoords);
    vec4 KdSampled = texture(uKdSampler, vTexCoords);
    vec4 KsSampled = texture(uKsSampler, vTexCoords);
    vec4 shininessSampled = texture(uShininessSampler, vTexCoords);
    vec3 Ka = mix(uKa, KaSampled.rgb, uKaSamplerFactor);
    vec3 Kd = mix(uKd, KdSampled.rgb, uKdSamplerFactor);
    vec3 Ks = mix(uKs, KsSampled.rgb, uKsSamplerFactor);
    float shininess = mix(uShininess, shininessSampled.r, uShininessSamplerFactor);

    vec3 color = Ka;

    // http://igm.univ-mlv.fr/~lnoel/index.php?section=teaching&teaching=opengl&teaching_section=tds&td=td8#intro

    vec3 N = normalize(vViewSpaceNormal);
    vec3 wo = vec3(0,0,1);

    vec3 wi = -uDirectionalLightDir; // Expected to be normalized
    vec3 Li = uDirectionalLightIntensity;
    vec3 halfVector = normalize(mix(wo, wi, 0.5));
    color += Li*(Kd*dot(wi, N) + pow(Ks*dot(halfVector, N), vec3(shininess)));

    float distFromPointLight = length(uPointLightPosition - vViewSpacePosition);
    wi = (uPointLightPosition - vViewSpacePosition) / distFromPointLight;
    Li = uPointLightIntensity / (uPointLightAttenuationFactor * pow(max(1, distFromPointLight / uPointLightRange), 2));
    halfVector = normalize(mix(wo, wi, 0.5));
    color += Li*(Kd*dot(wi, N) + pow(Ks*dot(halfVector, N), vec3(shininess)));

    fColor = vec4(color, 1); // FIXME make alpha vary
}
