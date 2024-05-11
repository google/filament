#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct spvDescriptorSetBuffer0
{
    sampler uSampler [[id(8)]];
    texture2d<float> uTex [[id(9)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], float4 uSub [[color(1)]])
{
    main0_out out = {};
    out.FragColor = uSub + spvDescriptorSet0.uTex.sample(spvDescriptorSet0.uSampler, float2(0.5));
    return out;
}

