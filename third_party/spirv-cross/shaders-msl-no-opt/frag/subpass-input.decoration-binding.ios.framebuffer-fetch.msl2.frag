#version 450

layout(set = 0, binding = 10, input_attachment_index = 1) uniform subpassInput uSub;
layout(location = 0) out vec4 FragColor;

layout(set = 0, binding = 9) uniform texture2D uTex;
layout(set = 0, binding = 8) uniform sampler uSampler;

void main()
{
	FragColor = subpassLoad(uSub) + texture(sampler2D(uTex, uSampler), vec2(0.5));
}
