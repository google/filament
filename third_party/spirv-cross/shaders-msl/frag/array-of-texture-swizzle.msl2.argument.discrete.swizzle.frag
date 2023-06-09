#version 450

layout(set = 0, binding = 1) uniform sampler2D uSampler0[4];
layout(set = 2, binding = 0) uniform sampler2D uSampler1;
layout(set = 1, binding = 4) uniform sampler2D uSamp;
layout(location = 0) in vec2 vUV;

layout(location = 0) out vec4 FragColor;

vec4 sample_in_func_1()
{
	return texture(uSampler0[2], vUV);
}

vec4 sample_in_func_2()
{
	return texture(uSampler1, vUV);
}

vec4 sample_single_in_func(sampler2D s)
{
	return texture(s, vUV);
}

void main()
{
	FragColor = sample_in_func_1();
	FragColor += sample_in_func_2();
	FragColor += sample_single_in_func(uSampler0[1]);
	FragColor += sample_single_in_func(uSampler1);
}
