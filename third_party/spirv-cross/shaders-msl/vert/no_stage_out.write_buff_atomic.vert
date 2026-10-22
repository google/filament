#version 450

layout(binding = 0, std430) coherent buffer _19_21
{
	uint _m0;
} _21;

layout(location = 0) in vec4 _14;

void main()
{
	gl_Position = _14;
	uint _26 = atomicAdd(_21._m0, 1u);
}

