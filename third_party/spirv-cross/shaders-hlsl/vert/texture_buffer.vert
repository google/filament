#version 450

layout(binding = 4) uniform samplerBuffer uSamp;
layout(rgba32f, binding = 5) uniform readonly imageBuffer uSampo;

void main()
{
   gl_Position = texelFetch(uSamp, 10) + imageLoad(uSampo, 100);
}
