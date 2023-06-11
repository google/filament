#version 310 es
#extension GL_EXT_texture_buffer : require

layout(binding = 4) uniform highp samplerBuffer uSamp;
layout(binding = 5, rgba32f) uniform readonly highp imageBuffer uSampo;

void main()
{
    gl_Position = texelFetch(uSamp, 10) + imageLoad(uSampo, 100);
}

