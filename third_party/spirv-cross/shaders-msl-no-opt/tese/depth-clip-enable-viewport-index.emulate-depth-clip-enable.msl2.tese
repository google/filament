#version 450
#extension GL_ARB_shader_viewport_layer_array : require
layout(quads) in;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	gl_ViewportIndex = 2;
}
