#version 450

layout(binding = 0) uniform sampler2D uSampler;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vec4(textureQueryLevels(uSampler));
}
