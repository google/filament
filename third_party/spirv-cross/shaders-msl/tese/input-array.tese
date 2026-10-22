#version 450

layout(ccw, quads, fractional_odd_spacing) in;
layout(location = 0) in vec4 Floats[];
layout(location = 2) in vec4 Floats2[gl_MaxPatchVertices];

void set_position()
{
	gl_Position = Floats[0] * gl_TessCoord.x + Floats2[1] * gl_TessCoord.y;
}

void main()
{
	set_position();
}
