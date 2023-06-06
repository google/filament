#version 450
layout(quads, ccw, equal_spacing) in;

layout(location = 0) patch in float P[4];

void main()
{
    gl_Position = vec4(P[0], P[1], P[2], P[3]);
}

