#version 450

layout(binding = 0, rgba32f) uniform writeonly imageBuffer RWTex;
layout(binding = 1) uniform samplerBuffer Tex;

layout(location = 0) out vec4 _entryPointOutput;

vec4 _main()
{
    vec4 storeTemp = vec4(1.0, 2.0, 3.0, 4.0);
    imageStore(RWTex, 20, storeTemp);
    return texelFetch(Tex, 10);
}

void main()
{
    vec4 _35 = _main();
    _entryPointOutput = _35;
}

