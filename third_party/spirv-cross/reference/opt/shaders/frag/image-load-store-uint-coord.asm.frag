#version 450

layout(binding = 1, rgba32f) uniform image2D RWIm;
layout(binding = 0, rgba32f) uniform writeonly imageBuffer RWBuf;
layout(binding = 1) uniform sampler2D ROIm;
layout(binding = 0) uniform samplerBuffer ROBuf;

layout(location = 0) out vec4 _entryPointOutput;

void main()
{
    imageStore(RWIm, ivec2(uvec2(10u)), vec4(10.0, 0.5, 8.0, 2.0));
    vec4 _70 = imageLoad(RWIm, ivec2(uvec2(30u)));
    imageStore(RWBuf, int(80u), _70);
    _entryPointOutput = (_70 + texelFetch(ROIm, ivec2(uvec2(50u, 60u)), 0)) + texelFetch(ROBuf, int(80u));
}

