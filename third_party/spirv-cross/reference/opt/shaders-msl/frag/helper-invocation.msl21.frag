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

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    float4 _52;
    if (!simd_is_helper_thread())
    {
        _52 = uSampler.sample(uSamplerSmplr, in.vUV, level(0.0));
    }
    else
    {
        _52 = float4(1.0);
    }
    out.FragColor = _52;
    return out;
}

