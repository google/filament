#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct SSBO
{
    float4 ssbo;
};

struct SSBOs
{
    float4 ssbo;
};

struct Push
{
    float4 push;
};

struct UBO
{
    float4 ubo;
};

struct UBOs
{
    float4 ubo;
};

struct spvDescriptorSetBuffer0
{
    texture2d<float> uTexture [[id(0)]];
    sampler uTextureSmplr [[id(1)]];
    array<texture2d<float>, 2> uTextures [[id(2)]];
    array<sampler, 2> uTexturesSmplr [[id(4)]];
    constant UBO* v_90 [[id(6)]];
};

struct spvDescriptorSetBuffer1
{
    array<texture2d<float>, 4> uTexture2 [[id(0)]];
    array<sampler, 2> uSampler [[id(4)]];
    device SSBO* v_60 [[id(6)]];
    const device SSBOs* ssbos [[id(7)]][2];
};

struct spvDescriptorSetBuffer2
{
    constant UBOs* ubos [[id(0)]][4];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

float4 sample_in_function2(thread texture2d<float> uTexture, thread const sampler uTextureSmplr, thread float2& vUV, thread const array<texture2d<float>, 4> uTexture2, thread const array<sampler, 2> uSampler, thread const array<texture2d<float>, 2> uTextures, thread const array<sampler, 2> uTexturesSmplr, device SSBO& v_60, const device SSBOs* constant (&ssbos)[2], constant Push& registers)
{
    float4 ret = uTexture.sample(uTextureSmplr, vUV);
    ret += uTexture2[2].sample(uSampler[1], vUV);
    ret += uTextures[1].sample(uTexturesSmplr[1], vUV);
    ret += v_60.ssbo;
    ret += ssbos[0]->ssbo;
    ret += registers.push;
    return ret;
}

float4 sample_in_function(thread texture2d<float> uTexture, thread const sampler uTextureSmplr, thread float2& vUV, thread const array<texture2d<float>, 4> uTexture2, thread const array<sampler, 2> uSampler, thread const array<texture2d<float>, 2> uTextures, thread const array<sampler, 2> uTexturesSmplr, device SSBO& v_60, const device SSBOs* constant (&ssbos)[2], constant Push& registers, constant UBO& v_90, constant UBOs* constant (&ubos)[4])
{
    float4 ret = sample_in_function2(uTexture, uTextureSmplr, vUV, uTexture2, uSampler, uTextures, uTexturesSmplr, v_60, ssbos, registers);
    ret += v_90.ubo;
    ret += ubos[0]->ubo;
    return ret;
}

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], constant spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(1)]], constant spvDescriptorSetBuffer2& spvDescriptorSet2 [[buffer(2)]], constant Push& registers [[buffer(3)]])
{
    main0_out out = {};
    out.FragColor = sample_in_function(spvDescriptorSet0.uTexture, spvDescriptorSet0.uTextureSmplr, in.vUV, spvDescriptorSet1.uTexture2, spvDescriptorSet1.uSampler, spvDescriptorSet0.uTextures, spvDescriptorSet0.uTexturesSmplr, (*spvDescriptorSet1.v_60), spvDescriptorSet1.ssbos, registers, (*spvDescriptorSet0.v_90), spvDescriptorSet2.ubos);
    out.FragColor += (*spvDescriptorSet0.v_90).ubo;
    out.FragColor += (*spvDescriptorSet1.v_60).ssbo;
    out.FragColor += spvDescriptorSet2.ubos[1]->ubo;
    out.FragColor += registers.push;
    return out;
}

