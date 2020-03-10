#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct spvDescriptorSetBuffer0
{
    texture2d<float> uTexture2 [[id(0)]];
    sampler uTexture2Smplr [[id(1)]];
    texture2d<float> uTexture1 [[id(2)]];
    sampler uTexture1Smplr [[id(3)]];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant spvDescriptorSetBuffer0& spvDescriptorSet0 [[buffer(0)]], texture2d<float> uTextureDiscrete2 [[texture(0)]], sampler uTextureDiscrete2Smplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = spvDescriptorSet0.uTexture2.sample(spvDescriptorSet0.uTexture2Smplr, in.vUV);
    out.FragColor += uTextureDiscrete2.sample(uTextureDiscrete2Smplr, in.vUV);
    return out;
}

