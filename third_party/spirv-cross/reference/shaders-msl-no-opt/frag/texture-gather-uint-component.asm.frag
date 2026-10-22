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

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSamp [[texture(0)]], sampler uSampSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uSamp.gather(uSampSmplr, in.vUV, int2(0), component::y);
    return out;
}

