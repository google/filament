#version 310 es
precision mediump float;
precision highp int;

uniform mediump ivec4 UBO1[2];
uniform mediump uvec4 UBO2[2];
uniform vec4 UBO0[2];
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = ((((vec4(UBO1[0]) + vec4(UBO1[1])) + vec4(UBO2[0])) + vec4(UBO2[1])) + UBO0[0]) + UBO0[1];
}

