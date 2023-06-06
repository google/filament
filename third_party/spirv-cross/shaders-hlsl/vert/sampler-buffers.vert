#version 450

layout(binding = 1) uniform samplerBuffer uFloatSampler;
layout(binding = 2) uniform isamplerBuffer uIntSampler;
layout(binding = 3) uniform usamplerBuffer uUintSampler;

vec4 sample_from_function(samplerBuffer s0, isamplerBuffer s1, usamplerBuffer s2)
{
	return texelFetch(s0, 20) +
		intBitsToFloat(texelFetch(s1, 40)) +
		uintBitsToFloat(texelFetch(s2, 60));
}

void main()
{
	gl_Position = sample_from_function(uFloatSampler, uIntSampler, uUintSampler);
}
