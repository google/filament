#version 450

layout(binding = 0, rgba8) uniform image2DMS uImage;
layout(binding = 1, rgba8) uniform image2DMSArray uImageArray;

void main()
{
    vec4 _29 = imageLoad(uImageArray, ivec3(1, 2, 4), 3);
    imageStore(uImage, ivec2(2, 3), 1, imageLoad(uImage, ivec2(1, 2), 2));
    imageStore(uImageArray, ivec3(2, 3, 7), 1, _29);
}

