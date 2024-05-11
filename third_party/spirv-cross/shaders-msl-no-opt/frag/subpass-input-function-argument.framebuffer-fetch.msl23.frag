#version 450

layout(set = 0, input_attachment_index = 0, binding = 0) uniform subpassInput uSub;
layout(location = 0) out vec4 FragColor;

vec4 samp3(subpassInput uS)
{
	return subpassLoad(uS);
}

vec4 samp2(subpassInput uS)
{
	return subpassLoad(uS) + samp3(uS);
}

vec4 samp()
{
	return subpassLoad(uSub) + samp3(uSub);
}

void main()
{
	FragColor = samp() + samp2(uSub);
}
