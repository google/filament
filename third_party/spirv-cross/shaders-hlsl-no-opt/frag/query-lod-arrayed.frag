#version 450

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec2 FragCoord;
layout(binding = 0) uniform texture2DArray uTex;
layout(binding = 1) uniform sampler uSamp;

void main()
{
	FragCoord = textureQueryLod(sampler2DArray(uTex, uSamp), vUV);
}
