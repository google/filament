#version 450
layout(quads, ccw, fractional_odd_spacing) in;

layout(location = 0) in vec4 Floats[];
layout(location = 2) in vec4 Floats2[];

void main()
{
    gl_Position = (Floats[0] * gl_TessCoord.x) + (Floats2[1] * gl_TessCoord.y);
}

