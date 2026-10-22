// RUN: %dxc -T ps_6_6 -E main %s -spirv | FileCheck %s

// CHECK-DAG: OpDecorate %Texture DescriptorSet 2
// CHECK-DAG: OpDecorate %Texture Binding 1
[[vk::binding(1, 2)]]
Texture2D Texture;

// CHECK-DAG: OpDecorate %SamplerDescriptorHeap DescriptorSet 0
// CHECK-DAG: OpDecorate %SamplerDescriptorHeap Binding 0

float4 main() : SV_Target {
  SamplerState Sampler = SamplerDescriptorHeap[5];
  // CHECK:     [[ptr:%[0-9]+]] = OpAccessChain %_ptr_UniformConstant_type_sampler %SamplerDescriptorHeap %uint_5
  // CHECK: [[sampler:%[0-9]+]] = OpLoad %type_sampler [[ptr]]
  // CHECK:   [[image:%[0-9]+]] = OpLoad %type_2d_image %Texture
  // CHECK:     [[tmp:%[0-9]+]] = OpSampledImage %type_sampled_image [[image]] [[sampler]]
  // CHECK:                       OpImageSampleImplicitLod %v4float [[tmp]]
  return Texture.Sample(Sampler, float2(0, 0));
}
