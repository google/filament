#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uShadow [[texture(0)]], depth2d<float> uTexture [[texture(1)]], sampler uShadowSmplr [[sampler(0)]], sampler uSampler [[sampler(2)]])
{
    main0_out out = {};
    out.FragColor = uShadow.sample_compare(uShadowSmplr, in.vUV.xy, in.vUV.z) + uTexture.sample_compare(uSampler, in.vUV.xy, in.vUV.z);
    return out;
}

