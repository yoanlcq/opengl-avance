#version 430

layout(local_size_x = 32, local_size_y = 32) in;
layout(rgba32f) uniform readonly restrict image2D uInputImage;
uniform writeonly restrict image2D uOutputImage;
uniform float uGammaExponent;

void main() {
    ivec2 inputImageSize = imageSize(uInputImage);
    ivec2 pixelCoords = ivec2(gl_GlobalInvocationID.xy);
    if(pixelCoords.x >= inputImageSize.x || pixelCoords.y >= inputImageSize.y)
        return;
    vec4 pixel = imageLoad(uInputImage, pixelCoords);
    pixel.rgb = pow(pixel.rgb, vec3(uGammaExponent));
    imageStore(uOutputImage, pixelCoords, pixel);
}
