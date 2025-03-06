// RUN: %dxc -T ps_6_6 -spirv %s | FileCheck %s

// CHECK-DAG:                                          OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                          OpExtension "SPV_EXT_descriptor_indexing"

// CHECK-DAG:            [[float_00:%[_a-zA-Z0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK-DAG:           [[sampler_t:%[_a-zA-Z0-9]+]] = OpTypeSampler
// CHECK-DAG:             [[image_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 2D 2 0 0 1 Unknown
// CHECK-DAG:        [[ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[sampler_t]]
// CHECK-DAG:     [[ptr_u_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[sampler_t]]
// CHECK-DAG:       [[ptr_u_image_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image_t]]
// CHECK-DAG:  [[ptr_u_ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_sampler_t]]

// CHECK-DAG:   [[sampler_heap:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_sampler_t]] UniformConstant
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] DescriptorSet 0
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] Binding 0
// CHECK-DAG:                                     OpDecorate [[texture:%[_a-zA-Z0-9]+]] DescriptorSet 3
// CHECK-DAG:                                     OpDecorate [[texture]] Binding 2

// CHECK-DAG:                       [[texture]] = OpVariable [[ptr_u_image_t]] UniformConstant

[[vk::binding(/* binding= */ 2, /* set= */ 3)]]
Texture2D Texture;

float4 main() : SV_Target {
  SamplerState Sampler = SamplerDescriptorHeap[2];
// CHECK:         [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_sampler_t]] [[sampler_heap]] %uint_2
// CHECK-DAG: [[sampler:%[0-9]+]] = OpLoad [[sampler_t]] [[ptr]]
// CHECK-DAG:     [[tex:%[0-9]+]] = OpLoad [[image_t]] [[texture]]

  return Texture.Sample(Sampler, float2(0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage %type_sampled_image [[tex]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_00]] None
}
