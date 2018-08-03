#version 310 es
precision mediump float;

layout(constant_id = 10) const int Value = 2;
layout(binding = 0) uniform SpecConstArray
{
	vec4 samples[Value];
};

layout(location = 0) flat in int Index;
layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = samples[Index];
}

