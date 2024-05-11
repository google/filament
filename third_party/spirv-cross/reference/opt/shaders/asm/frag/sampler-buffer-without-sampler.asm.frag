#version 450

layout(binding = 0, rgba32f) uniform writeonly imageBuffer RWTex;
layout(binding = 1) uniform samplerBuffer Tex;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    imageStore(RWTex, 20, vec4(1.0, 2.0, 3.0, 4.0));
    _entryPointOutput = texelFetch(Tex, 10);
}

