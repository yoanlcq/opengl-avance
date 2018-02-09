#version 430

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f) uniform readonly restrict image2D uInputImage;
uniform writeonly restrict image2D uOutputImage;
uniform float uGammaExponent;
//uniform int uBlurMatrixHalfSide;
const int uBlurMatrixHalfSide = 4;

void main() {
    ivec2 inputImageSize = imageSize(uInputImage);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);

    if(pixelCoords.x >= inputImageSize.x || pixelCoords.y >= inputImageSize.y)
        return;

#if 1
    vec4 accum = vec4(0);
    int s = uBlurMatrixHalfSide;

    for(int dy=-s ; dy <= s ; ++dy) {
        for(int dx=-s ; dx <= s ; ++dx) {
            ivec2 coords = clamp(pixelCoords + ivec2(dx, dy), ivec2(0), inputImageSize-ivec2(1));
            accum += imageLoad(uInputImage, coords);
        }
    }
    vec4 pixel = accum / ((s+s+1)*(s+s+1));
#else
    vec4 pixel = imageLoad(uInputImage, pixelCoords);
#endif
    pixel.rgb = pow(pixel.rgb, vec3(uGammaExponent));
    imageStore(uOutputImage, pixelCoords, pixel);
}
