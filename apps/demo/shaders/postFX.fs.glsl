#version 430

uniform sampler2D uHiResTexture;
uniform sampler2D uLoResTexture;
uniform uvec2 uWindowSize;

// NOTE: Keep in sync with GLDemoPostFXProgram's constants
#define BLUR_NONE 1
#define BLUR_BOX 2
#define BLUR_RADIAL 3
uniform uint uBlurTechnique;

uniform int uBoxBlurMatrixHalfSide; // int, not uint, so we can negate it
uniform uint uRadialBlurNumSamples;
uniform float uRadialBlurMaxLength;

uniform bool uBloomEnabled;
uniform int uBloomMatrixHalfSide; // int, not uint, so we can negate it
uniform float uBloomTexelSkip;
uniform float uBloomThreshold;

out vec4 fColor;

void main() {
    vec4 pixel = vec4(0);
    vec2 texCoords = vec2(gl_FragCoord.xy) / vec2(uWindowSize.xy);

    switch(uBlurTechnique) {
    case BLUR_NONE:
        pixel = texture2D(uHiResTexture, texCoords);
        break;
    case BLUR_BOX:
        int s = uBoxBlurMatrixHalfSide;
        for(int dy=-s ; dy <= s ; ++dy) {
            for(int dx=-s ; dx <= s ; ++dx) {
                vec2 coords = clamp(texCoords + vec2(dx, dy)/vec2(uWindowSize.xy), 0, 1);
                pixel += texture2D(uLoResTexture, coords);
            }
        }
        s += s+1;
        pixel /= s*s;
        break;
    case BLUR_RADIAL: 
        // https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch27.html
        vec2 coords = texCoords;
        vec2 center = vec2(0.5f);
        vec2 velocityFromCenter = (center - coords) / float(uRadialBlurNumSamples);
        float upperLength = uRadialBlurMaxLength / float(uRadialBlurNumSamples);
        vec2 velocity = length(velocityFromCenter) < upperLength ? velocityFromCenter : upperLength * normalize(velocityFromCenter);
        vec4 accum = vec4(0);
        for(uint i = 0; i < uRadialBlurNumSamples; ++i, coords += velocity) {
            accum += texture2D(uLoResTexture, coords);
        }
        accum /= float(uRadialBlurNumSamples);
        pixel = mix(texture2D(uHiResTexture, texCoords), accum, 2f*length(coords-center));
        break;
    default:
        // Welp, it's obviously invalid. User forgot to set uBlurTechnique.
        pixel = vec4(1,0,1,1);
        break;
    }

    if(uBloomEnabled) {
        int s = uBloomMatrixHalfSide;

        vec3 bloom = vec3(0);
        for(int dy=-s ; dy <= s ; ++dy) {
            for(int dx=-s ; dx <= s ; ++dx) {
                vec2 coords = clamp(texCoords + uBloomTexelSkip*vec2(dx, dy)/vec2(uWindowSize.xy), 0, 1);
                vec4 fetched = texture2D(uLoResTexture, coords);
                bloom += smoothstep(uBloomThreshold, 1.0f, fetched.rgb);
            }
        }
        s += s+1;
        pixel.rgb += bloom / (s*s);
    }
    
    pixel = clamp(pixel, 0, 1);
    fColor = pixel;
}
