#version 450

layout(location = 0) out vec2 FragColor;
layout(location = 0, component = 0) in vec2 v0;
layout(location = 0, component = 2) in float v1;
in Vertex
{
    layout(location = 1, component = 2) float v3;
} _20;


void main()
{
    FragColor = (v0 + vec2(v1)) + vec2(_20.v3);
}

