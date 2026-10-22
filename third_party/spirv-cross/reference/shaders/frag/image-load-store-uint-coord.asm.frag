#version 450

layout(binding = 1, rgba32f) uniform image2D RWIm;
layout(binding = 0, rgba32f) uniform writeonly imageBuffer RWBuf;
layout(binding = 1) uniform sampler2D ROIm;
layout(binding = 0) uniform samplerBuffer ROBuf;

layout(location = 0) out vec4 _entryPointOutput;

vec4 _main()
{
    vec4 storeTemp = vec4(10.0, 0.5, 8.0, 2.0);
    imageStore(RWIm, ivec2(uvec2(10u)), storeTemp);
    vec4 v = imageLoad(RWIm, ivec2(uvec2(30u)));
    imageStore(RWBuf, int(80u), v);
    v += texelFetch(ROIm, ivec2(uvec2(50u, 60u)), 0);
    v += texelFetch(ROBuf, int(80u));
    return v;
}

void main()
{
    vec4 _62 = _main();
    _entryPointOutput = _62;
}

