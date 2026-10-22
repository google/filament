#version 450
layout(quads) in;

void main()
{
	gl_Position = gl_in[0].gl_Position;
}
