#version 460 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

layout(set=0, binding=0) uniform sampler2D Image0;

void main(void)
{
    const ivec2 offs[4] = {ivec2(0,0), ivec2(1,0), ivec2(1,1), ivec2(0,1)};
    outColor = textureGatherOffsets(Image0, inUv, offs);
}
