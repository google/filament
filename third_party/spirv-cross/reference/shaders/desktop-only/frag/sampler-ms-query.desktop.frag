#version 450

layout(binding = 0) uniform sampler2DMS uSampler;
layout(binding = 1) uniform sampler2DMSArray uSamplerArray;
layout(binding = 2, rgba8) uniform readonly writeonly image2DMS uImage;
layout(binding = 3, rgba8) uniform readonly writeonly image2DMSArray uImageArray;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(float(((textureSamples(uSampler) + textureSamples(uSamplerArray)) + imageSamples(uImage)) + imageSamples(uImageArray)));
}

