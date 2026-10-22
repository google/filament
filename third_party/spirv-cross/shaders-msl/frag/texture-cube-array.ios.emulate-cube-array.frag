#version 450

layout(set = 0, binding = 0) uniform samplerCube cubeSampler;
layout(set = 0, binding = 1) uniform samplerCubeArray cubeArraySampler;
layout(set = 0, binding = 2) uniform sampler2DArray texArraySampler;

layout(location = 0) in vec4 vUV;
layout(location = 0) out vec4 FragColor;

void main()
{
	vec4 a = texture(cubeSampler, vUV.xyz);
	vec4 b = texture(cubeArraySampler, vUV);
	vec4 c = texture(texArraySampler, vUV.xyz);
	FragColor = a + b + c;
}
