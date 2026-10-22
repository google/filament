#version 450

layout(set = 0, binding = 0) uniform sampler2DArray tex;
layout(location = 0) out mediump vec4 FragColor;
layout(location = 0) in vec3 coord;

void main()
{
	FragColor = textureGatherOffsets(tex, coord, ivec2[](ivec2(-8), ivec2(-8, 7), ivec2(7, -8), ivec2(7)), 1);
}
