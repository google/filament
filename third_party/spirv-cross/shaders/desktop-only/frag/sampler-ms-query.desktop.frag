#version 450

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2DMS uSampler;
layout(binding = 1) uniform sampler2DMSArray uSamplerArray;
layout(rgba8, binding = 2) uniform image2DMS uImage;
layout(rgba8, binding = 3) uniform image2DMSArray uImageArray;

void main()
{
	FragColor =
		vec4(
			textureSamples(uSampler) +
			textureSamples(uSamplerArray) +
			imageSamples(uImage) +
			imageSamples(uImageArray));
}
