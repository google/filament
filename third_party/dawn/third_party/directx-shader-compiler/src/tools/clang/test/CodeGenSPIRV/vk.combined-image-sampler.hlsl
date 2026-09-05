// RUN: %dxc -T ps_6_0 -E main -O0  %s -spirv | FileCheck %s

// CHECK: OpDecorate %tex0 DescriptorSet 1
// CHECK: OpDecorate %tex0 Binding 0
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
SamplerState sam0;
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
Texture2D<float4> tex0;

// CHECK: OpDecorate %tex1 DescriptorSet 0
// CHECK: OpDecorate %tex1 Binding 1
[[vk::combinedImageSampler]]
SamplerState sam1 : register(s1);
[[vk::combinedImageSampler]]
Texture2D<float4> tex1 : register(t1);

// CHECK: OpDecorate %tex2 DescriptorSet 0
// CHECK: OpDecorate %tex2 Binding 2
[[vk::combinedImageSampler]]
SamplerState sam2 : register(s2);
[[vk::combinedImageSampler]]
Texture2D<float4> tex2 : register(t2);

// CHECK: OpDecorate %sam3 DescriptorSet 0
// CHECK: OpDecorate %sam3 Binding 0
SamplerState sam3 : register(s0);

// CHECK: OpDecorate %tex4 DescriptorSet 0
// CHECK: OpDecorate %tex4 Binding 4
[[vk::combinedImageSampler]]
Texture2D tex4 : register(t4);
[[vk::combinedImageSampler]]
SamplerComparisonState sam4 : register(s4);

Texture2D<float4> getTexture(int i) {
  if (i == 0) return tex0;
  if (i == 1) return tex1;
  if (i == 2) return tex2;
  return tex0;
}

SamplerState getSampler(int i) {
  if (i == 0) return sam0;
  if (i == 1) return sam1;
  if (i == 2) return sam2;
  if (i == 3) return sam3;
  return sam0;
}

float4 sampleLevel(int i) {
  return getTexture(i).SampleLevel(getSampler(i), float2(1, 2), 10, 2);
}

float4 main(int3 offset: A) : SV_Target {
  float4 ret = 0;

// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex0
// CHECK: OpImageSampleExplicitLod %v4float [[tex0]]
  ret += sampleLevel(0);

// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1
// CHECK: OpImageSampleExplicitLod %v4float [[tex1]]
  ret += sampleLevel(1);

// CHECK: [[tex2:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex2
// CHECK: OpImageSampleExplicitLod %v4float [[tex2]]
  ret += sampleLevel(2);

// CHECK: [[tex4:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex4
// CHECK: OpImageSampleDrefImplicitLod %float [[tex4]]
  ret += tex4.SampleCmp(sam4, float2(1, 2), 10, 2);

// CHECK: [[tex0_0:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex0
// CHECK: [[img_extracted_from_tex0:%[a-zA-Z0-9_]+]] = OpImage %type_2d_image [[tex0_0]]
// CHECK: [[sam3:%[a-zA-Z0-9_]+]] = OpLoad %type_sampler %sam3
// CHECK: [[sampled_image:%[a-zA-Z0-9_]+]] = OpSampledImage %type_sampled_image [[img_extracted_from_tex0]] [[sam3]]
// CHECK: OpImageSampleExplicitLod %v4float [[sampled_image]]
  ret += getTexture(0).SampleLevel(getSampler(3), float2(1, 2), 10, 2);

  return ret;
}
