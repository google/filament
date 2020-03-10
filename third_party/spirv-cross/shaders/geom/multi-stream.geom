#version 450

layout(triangles) in;
layout(points, max_vertices = 2) out;

void main()
{
	gl_Position = gl_in[0].gl_Position;
	EmitStreamVertex(0);
	EndStreamPrimitive(0);
	gl_Position = gl_in[0].gl_Position + 2;
	EmitStreamVertex(1);
	EndStreamPrimitive(1);
}

