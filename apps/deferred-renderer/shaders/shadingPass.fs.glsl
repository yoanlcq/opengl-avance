#version 330

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
#define MAX_POINT_LIGHTS 32 // NOTE: Keep in sync with GLDeferredShadingPassProgram
uniform uint uPointLightCount;
uniform vec3 uPointLightPosition[MAX_POINT_LIGHTS];
uniform vec3 uPointLightIntensity[MAX_POINT_LIGHTS];
uniform float uPointLightRange[MAX_POINT_LIGHTS];
uniform float uPointLightAttenuationFactor[MAX_POINT_LIGHTS];

out vec4 fColor;

void main() {
    vec3 position = texelFetch(uGPosition,       ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 N        = texelFetch(uGNormal,         ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 Ka       = texelFetch(uGAmbient,        ivec2(gl_FragCoord.xy), 0).xyz;
    vec3 Kd       = texelFetch(uGDiffuse,        ivec2(gl_FragCoord.xy), 0).xyz;
    vec4 glossy   = texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0);
    vec3 Ks = glossy.xyz;
    float shininess = glossy.w;

    vec3 color = vec3(0,0,0);

    // http://igm.univ-mlv.fr/~lnoel/index.php?section=teaching&teaching=opengl&teaching_section=tds&td=td8#intro

    vec3 wo = vec3(0,0,1);

    vec3 wi = -uDirectionalLightDir; // Expected to be normalized
    vec3 Li = uDirectionalLightIntensity;
    vec3 halfVector = normalize(mix(wo, wi, 0.5));
    color += Li*(Kd*dot(wi, N) + pow(Ks*dot(halfVector, N), vec3(shininess)));

    for(uint i=0u ; i<uPointLightCount ; ++i) {
        float distFromPointLight = length(uPointLightPosition[i] - position);
        wi = (uPointLightPosition[i] - position) / distFromPointLight;
        Li = uPointLightIntensity[i] / (uPointLightAttenuationFactor[i] * pow(max(1, distFromPointLight / uPointLightRange[i]), 2));
        halfVector = normalize(mix(wo, wi, 0.5));
        color += Li*(Kd*dot(wi, N) + pow(Ks*dot(halfVector, N), vec3(shininess)));
    }

    fColor = vec4(color, 1); // FIXME make alpha vary

}
