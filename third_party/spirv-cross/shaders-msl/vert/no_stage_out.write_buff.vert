#version 450

layout(binding = 1, std430) writeonly buffer _33_35
{
	uvec4 _m0[1024];
} _35;

layout(binding = 0, std140) uniform _38_40
{
	uvec4 _m0[1024];
} _40;

layout(location = 0) in vec4 _14;

void main()
{
	gl_Position = _14;
	for (int _19 = 0; _19 < 1024; _19++)
	{
		_35._m0[_19] = _40._m0[_19];
	}
}

