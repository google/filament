// RUN: %dxc -T ps_6_0 -E main -auto-binding-space 77 -fcgl  %s -spirv | FileCheck %s

// Since both the descriptor set and binding are mentioned via "[[vk::binding]]",
// the "-auto-binding-space" has no effect:
//
// CHECK:      OpDecorate %sampler1 DescriptorSet 6
// CHECK-NEXT: OpDecorate %sampler1 Binding 5
[[vk::binding(5,6)]]
SamplerState sampler1 : register(s1, space2);

// Since the space is NOT provided in "[[vk::binding]]",
// we use the default space specified by "-auto-binding-space"
//
// CHECK:      OpDecorate %sampler2 DescriptorSet 77
// CHECK-NEXT: OpDecorate %sampler2 Binding 8
[[vk::binding(8)]]
SamplerState sampler2 : register(s4);

// Since both the descriptor set and binding are mentioned via ":register",
// the "-auto-binding-space" has no effect:
//
// CHECK:      OpDecorate %sampler3 DescriptorSet 9
// CHECK-NEXT: OpDecorate %sampler3 Binding 1
SamplerState sampler3 : register(s1, space9);

// Since the space is NOT provided in ":register",
// we use the default space specified by "-auto-binding-space"
//
// CHECK:      OpDecorate %sampler4 DescriptorSet 77
// CHECK-NEXT: OpDecorate %sampler4 Binding 3
SamplerState sampler4 : register(s3);

float4 main() : SV_Target {
  return 1.0;
}
