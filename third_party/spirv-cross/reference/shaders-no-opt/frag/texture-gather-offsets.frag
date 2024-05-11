#version 460

layout(binding = 0) uniform sampler2D Image0;

layout(location = 0) out vec4 outColor;
layout(location = 0) in vec2 inUv;

void main()
{
    outColor = textureGatherOffsets(Image0, inUv, ivec2[](ivec2(0), ivec2(1, 0), ivec2(1), ivec2(0, 1)));
}

