#version 310 es
precision mediump float;
precision highp int;

#ifndef SPIRV_CROSS_CONSTANT_ID_10
#define SPIRV_CROSS_CONSTANT_ID_10 2
#endif
const int Value = SPIRV_CROSS_CONSTANT_ID_10;

layout(binding = 0, std140) uniform SpecConstArray
{
    vec4 samples[Value];
} _15;

layout(location = 0) out vec4 FragColor;
layout(location = 0) flat in mediump int Index;

void main()
{
    FragColor = _15.samples[Index];
}

