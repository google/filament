#version 450

layout(binding = 0) uniform sampler1D uSampler1D;
layout(binding = 1) uniform sampler2D uSampler2D;
layout(binding = 2) uniform sampler2DArray uSampler2DArray;
layout(binding = 3) uniform sampler3D uSampler3D;
layout(binding = 4) uniform samplerCube uSamplerCube;
layout(binding = 5) uniform samplerCubeArray uSamplerCubeArray;
layout(binding = 6) uniform samplerBuffer uSamplerBuffer;
layout(binding = 7) uniform sampler2DMS uSamplerMS;
layout(binding = 8) uniform sampler2DMSArray uSamplerMSArray;
layout(binding = 9, r32f) uniform readonly writeonly image1D uImage1D;
layout(binding = 10, r32f) uniform readonly writeonly image2D uImage2D;
layout(binding = 11, r32f) uniform readonly writeonly image2DArray uImage2DArray;
layout(binding = 12, r32f) uniform readonly writeonly image3D uImage3D;
layout(binding = 13, r32f) uniform readonly writeonly imageCube uImageCube;
layout(binding = 14, r32f) uniform readonly writeonly imageCubeArray uImageCubeArray;
layout(binding = 15, r32f) uniform readonly writeonly imageBuffer uImageBuffer;
layout(binding = 16, r32f) uniform readonly writeonly image2DMS uImageMS;
layout(binding = 17, r32f) uniform readonly writeonly image2DMSArray uImageMSArray;

void main()
{
}

