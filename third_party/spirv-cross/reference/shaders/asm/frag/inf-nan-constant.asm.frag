#version 310 es
precision mediump float;
precision highp int;

layout(location = 0) out highp vec3 FragColor;

void main()
{
    FragColor = vec3(uintBitsToFloat(0x7f800000u), uintBitsToFloat(0xff800000u), uintBitsToFloat(0x7fc00000u));
}

