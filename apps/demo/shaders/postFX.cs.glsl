#version 430

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f) uniform readonly restrict image2D uInputImage;
uniform writeonly restrict image2D uOutputImage;

// NOTE: Keep in sync with GLDemoPostFXProgram's constants
#define BLUR_NONE 1
#define BLUR_BOX 2
#define BLUR_RADIAL 3
uniform uint uBlurTechnique;

uniform int uBoxBlurMatrixHalfSide; // int, not uint, so we can negate it
uniform uint uRadialBlurNumSamples;
uniform float uRadialBlurMaxLength;

uniform float uGammaExponent;
uniform vec3 uFinalTouchMul;
uniform vec3 uFinalTouchAdd;

void main() {
    ivec2 inputImageSize = imageSize(uInputImage);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    if(pixelCoords.x >= inputImageSize.x || pixelCoords.y >= inputImageSize.y)
        return;

    vec4 pixel = vec4(0);

    switch(uBlurTechnique) {
    case BLUR_NONE:
        pixel = imageLoad(uInputImage, pixelCoords);
        break;
    case BLUR_BOX:
        int s = uBoxBlurMatrixHalfSide;

        for(int dy=-s ; dy <= s ; ++dy) {
            for(int dx=-s ; dx <= s ; ++dx) {
                ivec2 coords = clamp(pixelCoords + ivec2(dx, dy), ivec2(0), inputImageSize-ivec2(1));
                pixel += imageLoad(uInputImage, coords);
            }
        }
        pixel /= ((s+s+1)*(s+s+1));
        break;
    case BLUR_RADIAL: 
        // https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch27.html
        vec2 coords = vec2(pixelCoords);
        vec2 center = vec2(inputImageSize)/2.0f;
        vec2 velocityFromCenter = (center - coords) / float(uRadialBlurNumSamples);
        float upperLength = uRadialBlurMaxLength / float(uRadialBlurNumSamples);
        vec2 velocity = length(velocityFromCenter) < upperLength ? velocityFromCenter : upperLength * normalize(velocityFromCenter);
        for(uint i = 0; i < uRadialBlurNumSamples; ++i, coords += velocity) {
            pixel += imageLoad(uInputImage, ivec2(round(coords)));
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
                vec4 loaded = imageLoad(uInputImage, coords);
                bloom += smoothstep(0.6f, 1.0f, loaded.rgb) * 2.0f;
            }
        }
        pixel.rgb += bloom / 4.0f;
    }
#endif
    
    pixel.rgb = pow(pixel.rgb, vec3(uGammaExponent));
    pixel.rgb = fma(pixel.rgb, uFinalTouchMul, uFinalTouchAdd);
    pixel = clamp(pixel, 0, 1);
    imageStore(uOutputImage, pixelCoords, pixel);
}
