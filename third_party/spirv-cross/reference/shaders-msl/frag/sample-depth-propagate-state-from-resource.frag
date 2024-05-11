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

static inline __attribute__((always_inline))
float sample_normal2(depth2d<float> tex, sampler uSampler, thread float3& vUV)
{
    return float4(tex.sample(uSampler, vUV.xy)).x;
}

static inline __attribute__((always_inline))
float sample_normal(depth2d<float> tex, sampler uSampler, thread float3& vUV)
{
    return sample_normal2(tex, uSampler, vUV);
}

static inline __attribute__((always_inline))
float sample_comp(depth2d<float> tex, thread float3& vUV, sampler uSamplerShadow)
{
    return tex.sample_compare(uSamplerShadow, vUV.xy, vUV.z);
}

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uTexture [[texture(0)]], sampler uSampler [[sampler(0)]], sampler uSamplerShadow [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = sample_normal(uTexture, uSampler, in.vUV);
    out.FragColor += sample_comp(uTexture, in.vUV, uSamplerShadow);
    return out;
}

