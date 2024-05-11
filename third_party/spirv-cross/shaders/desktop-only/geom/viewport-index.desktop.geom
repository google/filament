#version 450

layout(triangles) in;
layout(triangle_strip) out;
layout(max_vertices = 4) out;

void main()
{
	gl_ViewportIndex = 1;
}

