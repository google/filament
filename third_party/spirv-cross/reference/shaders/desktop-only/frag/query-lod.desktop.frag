#version 450

layout(binding = 0) uniform sampler2D uSampler;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vTexCoord;

void main()
{
    FragColor = textureQueryLod(uSampler, vTexCoord).xyxy;
}

