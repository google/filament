#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float vTex [[user(locn0), flat]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor += ((uSampler.sample(uSamplerSmplr, float2(in.vTex, 0.5), bias(2.0)) + uSampler.sample(uSamplerSmplr, float2(in.vTex, 0.5), level(3.0))) + uSampler.sample(uSamplerSmplr, float2(in.vTex, 0.5), gradient2d(5.0, 8.0)));
    return out;
}

