#version 450

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;
layout(set = 0, binding = 0) uniform sampler2D uSamplers[10000];
layout(set = 2, binding = 0) uniform sampler2D uSampler;

layout(set = 1, binding = 0) uniform UBO
{
	vec4 v;
} vs[10000];

vec4 samp_array()
{
	return texture(uSamplers[9999], vUV) + vs[5000].v;
}

vec4 samp_single()
{
	return texture(uSampler, vUV);
}

void main()
{
	FragColor = samp_array() + samp_single();
}
