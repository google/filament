#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

float sample_combined(thread float3& vUV, thread depth2d<float> uShadow, thread const sampler uShadowSmplr)
{
    return uShadow.sample_compare(uShadowSmplr, vUV.xy, vUV.z);
}

float sample_separate(thread float3& vUV, thread depth2d<float> uTexture, thread sampler uSampler)
{
    return uTexture.sample_compare(uSampler, vUV.xy, vUV.z);
}

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uShadow [[texture(0)]], depth2d<float> uTexture [[texture(1)]], sampler uShadowSmplr [[sampler(0)]], sampler uSampler [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = sample_combined(in.vUV, uShadow, uShadowSmplr) + sample_separate(in.vUV, uTexture, uSampler);
    return out;
}

