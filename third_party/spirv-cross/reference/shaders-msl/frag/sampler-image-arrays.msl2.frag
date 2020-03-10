#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vTex [[user(locn0), flat]];
    int vIndex [[user(locn1)]];
};

static inline __attribute__((always_inline))
float4 sample_from_global(thread int& vIndex, thread float2& vTex, thread const array<texture2d<float>, 4> uSampler, thread const array<sampler, 4> uSamplerSmplr)
{
    return uSampler[vIndex].sample(uSamplerSmplr[vIndex], (vTex + float2(0.100000001490116119384765625)));
}

static inline __attribute__((always_inline))
float4 sample_from_argument(thread const array<texture2d<float>, 4> samplers, thread const array<sampler, 4> samplersSmplr, thread int& vIndex, thread float2& vTex)
{
    return samplers[vIndex].sample(samplersSmplr[vIndex], (vTex + float2(0.20000000298023223876953125)));
}

static inline __attribute__((always_inline))
float4 sample_single_from_argument(thread const texture2d<float> samp, thread const sampler sampSmplr, thread float2& vTex)
{
    return samp.sample(sampSmplr, (vTex + float2(0.300000011920928955078125)));
}

fragment main0_out main0(main0_in in [[stage_in]], array<texture2d<float>, 4> uSampler [[texture(0)]], array<texture2d<float>, 4> uTextures [[texture(4)]], array<sampler, 4> uSamplerSmplr [[sampler(0)]], array<sampler, 4> uSamplers [[sampler(4)]])
{
    main0_out out = {};
    out.FragColor = float4(0.0);
    out.FragColor += uTextures[2].sample(uSamplers[1], in.vTex);
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], in.vTex);
    out.FragColor += sample_from_global(in.vIndex, in.vTex, uSampler, uSamplerSmplr);
    out.FragColor += sample_from_argument(uSampler, uSamplerSmplr, in.vIndex, in.vTex);
    out.FragColor += sample_single_from_argument(uSampler[3], uSamplerSmplr[3], in.vTex);
    return out;
}

