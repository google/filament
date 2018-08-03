#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in ivec4 vA;
layout(location = 1) flat in ivec4 vB;

void main()
{
    FragColor = vec4(vA - vB * (vA / vB));
}

