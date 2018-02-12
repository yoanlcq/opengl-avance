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
        for(uint i = 0; i < uRadialBlurNumSamples; ++i, coords += velocity) {
            pixel += texture2D(uLoResTexture, coords);
        }
        pixel /= float(uRadialBlurNumSamples);
        break;
    default:
        // Welp, it's obviously invalid. User forgot to set uBlurTechnique.
        pixel = vec4(1,0,1,1);
        break;
    }

#if 0 // Experimental bloom
        int s = 4;

        vec3 bloom = vec3(0);
        for(int dy=-s ; dy <= s ; ++dy) {
            for(int dx=-s ; dx <= s ; ++dx) {
                ivec2 coords = clamp(pixelCoords + ivec2(dx, dy), ivec2(0), inputImageSize-ivec2(1));
                vec4 loaded = imageLoad(uHiResTexture, coords);
                bloom += smoothstep(0.6f, 1.0f, loaded.rgb) * 2.0f;
            }
        }
        pixel.rgb += bloom / 4.0f;
    }
#endif
    
    pixel = clamp(pixel, 0, 1);
    fColor = pixel;
}
