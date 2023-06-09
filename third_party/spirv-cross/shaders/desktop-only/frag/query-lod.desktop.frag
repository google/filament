#version 450

layout(location = 0) in vec2 vTexCoord;
layout(binding = 0) uniform sampler2D uSampler;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = textureQueryLod(uSampler, vTexCoord).xyxy;
}
