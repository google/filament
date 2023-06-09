#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d<float> uDepth [[texture(0)]], sampler uSampler [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uDepth.sample(uSampler, float2(0.5));
    return out;
}

