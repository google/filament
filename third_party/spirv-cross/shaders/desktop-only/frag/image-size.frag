#version 450

layout(location = 0) out vec4 FragColor;
layout(r32f, set = 0, binding = 0) uniform image2D uImage1;
layout(r32f, set = 0, binding = 1) uniform image2D uImage2;

void main()
{
	FragColor = vec4(imageSize(uImage1), imageSize(uImage2));
}
