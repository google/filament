#version 450

layout(location = 0) out vec4 color;

void main()
{
	color = vec4(1.0);
	gl_FragDepth = 1.25;
}
