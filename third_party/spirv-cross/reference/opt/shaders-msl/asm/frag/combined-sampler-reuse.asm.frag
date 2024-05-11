#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uTex [[texture(0)]], sampler uSampler [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uTex.sample(uSampler, in.vUV);
    out.FragColor += uTex.sample(uSampler, in.vUV, int2(1));
    return out;
}

