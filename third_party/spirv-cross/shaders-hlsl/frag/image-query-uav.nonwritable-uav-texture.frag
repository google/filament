#version 450

layout(rgba32f, binding = 0) uniform writeonly image1D uImage1D;
layout(rg32f, binding = 1) uniform writeonly image2D uImage2D;
layout(r32f, binding = 2) uniform readonly image2DArray uImage2DArray;
layout(rgba8, binding = 3) uniform writeonly image3D uImage3D;
layout(rgba8_snorm, binding = 6) uniform writeonly imageBuffer uImageBuffer;

// There is no RWTexture2DMS.

void main()
{
	int a = imageSize(uImage1D);
	ivec2 b = imageSize(uImage2D);
	ivec3 c = imageSize(uImage2DArray);
	ivec3 d = imageSize(uImage3D);
	int e = imageSize(uImageBuffer);
}
