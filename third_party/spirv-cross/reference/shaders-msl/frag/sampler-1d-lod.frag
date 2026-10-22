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

fragment main0_out main0(main0_in in [[stage_in]], texture1d<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor += ((uSampler.sample(uSamplerSmplr, in.vTex) + uSampler.sample(uSamplerSmplr, in.vTex)) + uSampler.sample(uSamplerSmplr, in.vTex));
    return out;
}

