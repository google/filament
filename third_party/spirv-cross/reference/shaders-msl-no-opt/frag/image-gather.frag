#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSamp [[texture(0)]], depth2d<float> uSampShadow [[texture(1)]], sampler uSampSmplr [[sampler(0)]], sampler uSampShadowSmplr [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = uSamp.gather(uSampSmplr, in.vUV.xy, int2(0), component::x);
    out.FragColor += uSamp.gather(uSampSmplr, in.vUV.xy, int2(0), component::y);
    out.FragColor += uSampShadow.gather_compare(uSampShadowSmplr, in.vUV.xy, in.vUV.z);
    return out;
}

