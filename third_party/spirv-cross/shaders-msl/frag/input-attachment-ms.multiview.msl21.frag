#version 450

layout(input_attachment_index = 0, set = 0, binding = 0) uniform subpassInputMS uSubpass0;
layout(input_attachment_index = 1, set = 0, binding = 1) uniform subpassInputMS uSubpass1;
layout(location = 0) out vec4 FragColor;

vec4 load_subpasses(mediump subpassInputMS uInput)
{
	return subpassLoad(uInput, gl_SampleID);
}

void main()
{
    FragColor = subpassLoad(uSubpass0, 1) + subpassLoad(uSubpass1, 2) + load_subpasses(uSubpass0);
}
