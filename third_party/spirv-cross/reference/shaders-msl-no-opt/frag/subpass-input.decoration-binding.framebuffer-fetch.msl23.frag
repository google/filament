#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(float4 uSub [[color(1)]], texture2d<float> uTex [[texture(9)]], sampler uSampler [[sampler(8)]])
{
    main0_out out = {};
    out.FragColor = uSub + uTex.sample(uSampler, float2(0.5));
    return out;
}

