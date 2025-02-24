// RUN: %dxc -T ps_6_6 -Od -spirv %s | FileCheck %s

// CHECK-DAG:                                          OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                          OpExtension "SPV_EXT_descriptor_indexing"

// CHECK-DAG:            [[float_00:%[_a-zA-Z0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK-DAG:           [[sampler_t:%[_a-zA-Z0-9]+]] = OpTypeSampler
// CHECK-DAG:             [[image_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 2D 2 0 0 1 Unknown
// CHECK-DAG:        [[ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[sampler_t]]
// CHECK-DAG:          [[ra_image_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image_t]]
// CHECK-DAG:     [[ptr_u_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[sampler_t]]
// CHECK-DAG:       [[ptr_u_image_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image_t]]
// CHECK-DAG:  [[ptr_u_ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_sampler_t]]
// CHECK-DAG:    [[ptr_u_ra_image_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image_t]]

// CHECK-DAG:   [[sampler_heap:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_sampler_t]] UniformConstant
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] DescriptorSet 0
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] Binding 1
// CHECK-DAG:  [[resource_heap:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_image_t]] UniformConstant
// CHECK-DAG:                                     OpDecorate [[resource_heap]] DescriptorSet 0
// CHECK-DAG:                                     OpDecorate [[resource_heap]] Binding 0

static SamplerState Sampler = SamplerDescriptorHeap[2];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_sampler_t]] [[sampler_heap]] %uint_2
// CHECK: [[sampler:%[0-9]+]] = OpLoad [[sampler_t]] [[ptr]]

static Texture2D Texture = ResourceDescriptorHeap[3];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_image_t]] [[resource_heap]] %uint_3
// CHECK: [[texture:%[0-9]+]] = OpLoad [[image_t]] [[ptr]]

float4 main() : SV_Target {

  return Texture.Sample(Sampler, float2(0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage %type_sampled_image [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_00]] None
}
