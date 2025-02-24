// RUN: %dxc -T ps_6_0 -E main -fvk-t-shift 10 0 -fvk-s-shift 20 0 -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %sam DescriptorSet 0
// CHECK: OpDecorate %sam Binding 10
[[vk::combinedImageSampler]]
SamplerState sam : register(s0);

// CHECK: OpDecorate %tex DescriptorSet 0
// CHECK: OpDecorate %tex Binding 10
[[vk::combinedImageSampler]]
Texture2D<float4> tex : register(t0);

// CHECK: OpDecorate %sam0 DescriptorSet 0
// CHECK: OpDecorate %sam0 Binding 20
SamplerState sam0 : register(s0);

// CHECK: OpDecorate %sam1 DescriptorSet 0
// CHECK: OpDecorate %sam1 Binding 21
SamplerState sam1 : register(s1);

float4 main(float4 P : SV_POSITION) : SV_TARGET
{
    return tex.Sample(sam, P.xy);
}