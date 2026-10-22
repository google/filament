#version 450
layout(quads, ccw, equal_spacing) in;

layout(location = 0) in vec4 vTexCoord[][1];

void main()
{
    gl_Position = (vTexCoord[0u][0u] + vTexCoord[2u][0u]) + vTexCoord[3u][0u];
}

