#version 450

layout(location = 0) out vec4 FragColors[2];
layout(location = 2) out vec4 FragColor;
const vec4 _3_init[2] = vec4[](vec4(1.0, 2.0, 3.0, 4.0), vec4(10.0));
const vec4 _4_init = vec4(5.0);

void main()
{
    FragColors = _3_init;
    FragColor = _4_init;
}

