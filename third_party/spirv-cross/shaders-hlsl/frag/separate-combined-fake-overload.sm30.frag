#version 450

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform sampler2D uSamp;
layout(binding = 1) uniform texture2D uT;
layout(binding = 2) uniform sampler uS;

vec4 samp(sampler2D uSamp)
{
	return texture(uSamp, vec2(0.5));
}

vec4 samp(texture2D T, sampler S)
{
	return texture(sampler2D(T, S), vec2(0.5));
}

void main()
{
	FragColor = samp(uSamp) + samp(uT, uS);
}
