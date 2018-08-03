#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

float4 samp(thread const texture2d<float> t, thread const sampler s)
{
    return t.sample(s, float2(0.5));
}

fragment main0_out main0(texture2d<float> uDepth [[texture(1)]], sampler uSampler [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = samp(uDepth, uSampler);
    return out;
}

