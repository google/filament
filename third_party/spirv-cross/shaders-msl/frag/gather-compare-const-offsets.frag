#version 450

layout(set = 0, binding = 0) uniform sampler2DShadow tex;
layout(location = 0) out mediump vec4 FragColor;
layout(location = 0) in vec2 coord;
layout(location = 1) in vec2 compare_value;

void main()
{
	FragColor = textureGatherOffsets(tex, coord, compare_value.x, ivec2[](ivec2(-8, 3), ivec2(-4, 7), ivec2(0, 3), ivec2(3, 0)));
}
