#version 310 es
#extension GL_OES_texture_buffer : require

layout(binding = 4) uniform highp samplerBuffer uSamp;
layout(rgba32f, binding = 5) uniform readonly highp imageBuffer uSampo;

void main()
{
   gl_Position = texelFetch(uSamp, 10) + imageLoad(uSampo, 100);
}
