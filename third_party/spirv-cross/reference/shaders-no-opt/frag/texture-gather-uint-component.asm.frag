#version 450

layout(binding = 0) uniform sampler2D uSamp;

layout(location = 0) out vec4 FragColor;
layout(location = 0) in vec2 vUV;

void main()
{
    FragColor = textureGather(uSamp, vUV, int(1u));
}

