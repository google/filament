#version 310 es
precision mediump float;
precision highp int;

layout(binding = 0, std140) uniform SpecConstArray
{
    vec4 samples[2];
} _15;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int Index;

void main()
{
    FragColor = _15.samples[Index];
}

