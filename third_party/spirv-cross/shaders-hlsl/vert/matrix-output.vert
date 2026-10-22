#version 450

layout(location = 0) out mat4 m;

void main()
{
	gl_Position = vec4(1.0);
	m = mat4(1.0);
}
