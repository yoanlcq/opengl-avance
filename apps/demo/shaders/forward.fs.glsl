#version 330

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
#define MAX_POINT_LIGHTS 32 // NOTE: Keep in sync with CommonLighting
uniform uint uPointLightCount;
uniform vec3 uPointLightPosition[MAX_POINT_LIGHTS];
uniform vec3 uPointLightIntensity[MAX_POINT_LIGHTS];
uniform float uPointLightRange[MAX_POINT_LIGHTS];
uniform float uPointLightAttenuationFactor[MAX_POINT_LIGHTS];

// NOTE: These are not used yet: Shadow mapping in forward rendering pipeline
// is not implemented yet.
uniform mat4 uDirLightViewProjMatrix;
uniform sampler2DShadow uDirLightShadowMap;
uniform float uDirLightShadowMapBias;
uniform int uDirLightShadowMapSampleCount;
uniform float uDirLightShadowMapSpread;


uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;
uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

out vec4 fColor;

const vec2 poissonDisk[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);

float random(vec4 seed) {
    float dot_product = dot(seed, vec4(12.9898,78.233,45.164,94.673));
    return fract(sin(dot_product) * 43758.5453);
}


void main() {

    vec3 Ka = uKa * texture(uKaSampler, vTexCoords).rgb;
    vec3 Kd = uKd * texture(uKdSampler, vTexCoords).rgb;
    vec3 Ks = uKs * texture(uKsSampler, vTexCoords).rgb;
    float shininess = uShininess * texture(uShininessSampler, vTexCoords).r;

    vec4 positionInDirLightScreen = uDirLightViewProjMatrix * vec4(vViewSpacePosition, 1); // Compute fragment position in NDC space of light
    vec3 positionInDirLightNDC = vec3(positionInDirLightScreen / positionInDirLightScreen.w) * 0.5 + 0.5; // Homogeneize + put between 0 and 1
    // float dirLightVisibility = textureProj(uDirLightShadowMap, vec4(positionInDirLightNDC.xy, positionInDirLightNDC.z - uDirLightShadowMapBias, 1.0), 0.0);

    float dirLightVisibility = 0.0;
    float dirSampleCountf = float(uDirLightShadowMapSampleCount);
    int step = max(1, 16 / uDirLightShadowMapSampleCount);
    for (int i = 0; i < uDirLightShadowMapSampleCount; ++i) {
#ifdef NOISY_SHADOWS
        int index = int(dirSampleCountf * random(vec4(gl_FragCoord.xyy, i))) % uDirLightShadowMapSampleCount;
#else // Blurred shadows
        int index = (i + step) % uDirLightShadowMapSampleCount;
#endif

        dirLightVisibility += textureProj(uDirLightShadowMap, vec4(positionInDirLightNDC.xy + uDirLightShadowMapSpread * poissonDisk[index], positionInDirLightNDC.z - uDirLightShadowMapBias, 1.0), 0.0);
    }
    dirLightVisibility /= dirSampleCountf;


    vec3 color = Ka;
    const vec3 SMALL3 = vec3(0.001f);

    // http://igm.univ-mlv.fr/~lnoel/index.php?section=teaching&teaching=opengl&teaching_section=tds&td=td8#intro

    vec3 N = normalize(vViewSpaceNormal);
    vec3 wo = vec3(0,0,1);

    vec3 wi = -uDirectionalLightDir; // Expected to be normalized
    vec3 Li = uDirectionalLightIntensity;
    vec3 halfVector = normalize(mix(wo, wi, 0.5));
    color += dirLightVisibility * Li*(Kd*dot(wi, N) + pow(max(vec3(0), Ks*dot(halfVector, N)), max(SMALL3, vec3(shininess))));

    for(uint i=0u ; i<uPointLightCount ; ++i) {
        float distFromPointLight = length(uPointLightPosition[i] - vViewSpacePosition);
        wi = (uPointLightPosition[i] - vViewSpacePosition) / distFromPointLight;
        Li = uPointLightIntensity[i] / (uPointLightAttenuationFactor[i] * pow(max(1, distFromPointLight / uPointLightRange[i]), 2));
        halfVector = normalize(mix(wo, wi, 0.5));
        color += Li*(Kd*dot(wi, N) + pow(max(vec3(0), Ks*dot(halfVector, N)), max(SMALL3, vec3(shininess))));
    }

    fColor = vec4(color, 1); // FIXME make alpha vary
}
