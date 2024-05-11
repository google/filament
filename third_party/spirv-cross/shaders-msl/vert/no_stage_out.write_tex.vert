#version 450

layout(binding = 1, r32ui) uniform writeonly uimage1D _32;
layout(binding = 0, r32ui) uniform readonly uimage1D _35;

layout(location = 0) in vec4 _14;

void main()
{
	gl_Position = _14;
	for (int _19 = 0; _19 < 128; _19++)
	{
		imageStore(_32, _19, imageLoad(_35, _19));
	}
}

