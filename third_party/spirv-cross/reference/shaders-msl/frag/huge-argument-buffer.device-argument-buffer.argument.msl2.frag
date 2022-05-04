#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 v;
};

struct spvDescriptorSetBuffer0
{
    array<texture2d<float>, 10000> uSamplers [[id(0)]];
    array<sampler, 10000> uSamplersSmplr [[id(10000)]];
};

struct spvDescriptorSetBuffer1
{
    constant UBO* vs [[id(0)]][10000];
};

struct spvDescriptorSetBuffer2
{
    texture2d<float> uSampler [[id(0)]];
    sampler uSamplerSmplr [[id(1)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

static inline __attribute__((always_inline))
float4 samp_array(const device array<texture2d<float>, 10000>& uSamplers, const device array<sampler, 10000>& uSamplersSmplr, thread float2& vUV, constant UBO* const device (&vs)[10000])
{
    return uSamplers[9999].sample(uSamplersSmplr[9999], vUV) + vs[5000]->v;
}

static inline __attribute__((always_inline))
float4 samp_single(thread float2& vUV, texture2d<float> uSampler, sampler uSamplerSmplr)
{
    return uSampler.sample(uSamplerSmplr, vUV);
}

fragment main0_out main0(main0_in in [[stage_in]], const device spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], const device spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(1)]], constant spvDescriptorSetBuffer2& spvDescriptorSet2 [[buffer(2)]])
{
    main0_out out = {};
    out.FragColor = samp_array(spvDescriptorSet0.uSamplers, spvDescriptorSet0.uSamplersSmplr, in.vUV, spvDescriptorSet1.vs) + samp_single(in.vUV, spvDescriptorSet2.uSampler, spvDescriptorSet2.uSamplerSmplr);
    return out;
}

