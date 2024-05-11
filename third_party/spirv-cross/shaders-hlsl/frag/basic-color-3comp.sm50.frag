#version 310 es
precision mediump float;

layout(location = 0) in vec4 vColor;
layout(location = 0) out vec3 FragColor;

void main()
{
    FragColor = vColor.xyz;
}

