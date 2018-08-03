#version 310 es
precision mediump float;
precision highp int;

struct PushConstants
{
    vec4 value0;
    vec4 value1;
};

uniform PushConstants push;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec4 vColor;

void main()
{
    FragColor = (vColor + push.value0) + push.value1;
}

