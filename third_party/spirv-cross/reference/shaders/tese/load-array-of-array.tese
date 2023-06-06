#version 450
layout(quads, ccw, equal_spacing) in;

layout(location = 0) in vec4 vTexCoord[][1];

void main()
{
    vec4 _17_unrolled[32][1];
    for (int i = 0; i < int(32); i++)
    {
        _17_unrolled[i] = vTexCoord[i];
    }
    vec4 tmp[32][1] = _17_unrolled;
    gl_Position = (tmp[0][0] + tmp[2][0]) + tmp[3][0];
}

