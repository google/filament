#version 450

layout(binding = 0) uniform sampler2D uT;
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = textureGather(uT, vec2(0.5), 3);
}
