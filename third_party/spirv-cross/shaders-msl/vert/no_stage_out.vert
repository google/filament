#version 450

layout(binding = 0, std430) writeonly buffer _10_12
{
	uvec4 _m0[1024];
} _12;

layout(location = 0) in uvec4 _19;

void main()
{
	_12._m0[gl_VertexIndex] = _19;
}

