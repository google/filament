#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vA;
layout(location = 1) in vec4 vB;

void main()
{
    if (gl_FrontFacing)
    {
        FragColor = vA;
    }
    else
    {
        FragColor = vB;
    }
}

