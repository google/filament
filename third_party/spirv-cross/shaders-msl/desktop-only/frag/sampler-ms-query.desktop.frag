#version 450

layout(binding = 0) uniform sampler2DMS uSampler;
layout(binding = 2, rgba8) uniform readonly writeonly image2DMS uImage;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(float(textureSamples(uSampler) + imageSamples(uImage)));
}

