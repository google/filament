#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vIn;
layout(location = 1) flat in mediump ivec4 vIn1;

void main()
{
    FragColor = -(-vIn);
    mediump ivec4 a = ~(~vIn1);
    bool b = false;
    b = !(!b);
}

