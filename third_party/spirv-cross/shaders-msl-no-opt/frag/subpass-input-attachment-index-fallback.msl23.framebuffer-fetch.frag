#version 450

layout(binding = 4, input_attachment_index = 1) uniform subpassInput uInput;
layout(location = 1) out vec4 FragColor;

void main()
{
	FragColor = subpassLoad(uInput);
}
