#version 450

layout(location = 0) out vec4 FragColors[2];
layout(location = 2) out vec4 FragColor;

void main()
{
    FragColors = vec4[](vec4(1.0, 2.0, 3.0, 4.0), vec4(10.0));
    FragColor = vec4(5.0);
}

