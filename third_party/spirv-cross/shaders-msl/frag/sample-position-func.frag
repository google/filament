#version 450

layout(location = 0) in flat int index;

layout(location = 0) out vec4 FragColor;

vec4 getColor(int i)
{
	return vec4(gl_SamplePosition, i, 1.0);
}

void main()
{
	FragColor = getColor(index);
}
