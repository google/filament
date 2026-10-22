#version 450

layout(location = 0) out vec4 f;
layout(location = 0) in vec4 f2;

void main()
{
	gl_Position = modf(f2, f);
}
