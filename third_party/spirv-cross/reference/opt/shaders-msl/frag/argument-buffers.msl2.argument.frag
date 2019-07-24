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
    constant UBO* m_90 [[id(6)]];
};

struct spvDescriptorSetBuffer1
{
    array<texture2d<float>, 4> uTexture2 [[id(0)]];
    array<sampler, 2> uSampler [[id(4)]];
    device SSBO* m_60 [[id(6)]];
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

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], constant spvDescriptorSetBuffer1& spvDescriptorSet1 [[buffer(1)]], constant spvDescriptorSetBuffer2& spvDescriptorSet2 [[buffer(2)]], constant Push& registers [[buffer(3)]])
{
    main0_out out = {};
    out.FragColor = ((((((spvDescriptorSet0.uTexture.sample(spvDescriptorSet0.uTextureSmplr, in.vUV) + spvDescriptorSet1.uTexture2[2].sample(spvDescriptorSet1.uSampler[1], in.vUV)) + spvDescriptorSet0.uTextures[1].sample(spvDescriptorSet0.uTexturesSmplr[1], in.vUV)) + (*spvDescriptorSet1.m_60).ssbo) + spvDescriptorSet1.ssbos[0]->ssbo) + registers.push) + (*spvDescriptorSet0.m_90).ubo) + spvDescriptorSet2.ubos[0]->ubo;
    out.FragColor += (*spvDescriptorSet0.m_90).ubo;
    out.FragColor += (*spvDescriptorSet1.m_60).ssbo;
    out.FragColor += spvDescriptorSet2.ubos[1]->ubo;
    out.FragColor += registers.push;
    return out;
}

