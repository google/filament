#version 450

layout(binding = 0, r32f) uniform image2D uImage1;
layout(binding = 1, r32f) uniform image2D uImage2;

layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = vec4(vec2(imageSize(uImage1)), vec2(imageSize(uImage2)));
}

