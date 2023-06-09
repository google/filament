#version 450

layout(rgba8, binding = 0) uniform image2D uImage;
layout(rgba8, binding = 1) uniform image2DArray uImageArray;
layout(rgba8, binding = 2) uniform image2DMS uImageMS;

void main()
{
	vec4 a = imageLoad(uImageMS, ivec2(1, 2), 2);
	vec4 b = imageLoad(uImageArray, ivec3(1, 2, 4));
	imageStore(uImage, ivec2(2, 3), a);
	imageStore(uImageArray, ivec3(2, 3, 7), b);
}
