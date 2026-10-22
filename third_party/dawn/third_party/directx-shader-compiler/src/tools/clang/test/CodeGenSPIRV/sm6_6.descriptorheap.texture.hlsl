// RUN: %dxc -T ps_6_6 -spirv %s | FileCheck %s

// CHECK-DAG:            [[float_00:%[_a-zA-Z0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK-DAG:           [[float_000:%[_a-zA-Z0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0

// CHECK-DAG:           [[sampler_t:%[_a-zA-Z0-9]+]] = OpTypeSampler
// CHECK-DAG:        [[ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[sampler_t]]
// CHECK-DAG:     [[ptr_u_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[sampler_t]]
// CHECK-DAG:  [[ptr_u_ra_sampler_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_sampler_t]]

// CHECK-DAG:             [[image1_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 1D 2 0 0 1 Unknown
// CHECK-DAG:          [[ra_image1_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image1_t]]
// CHECK-DAG:       [[ptr_u_image1_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image1_t]]
// CHECK-DAG:    [[ptr_u_ra_image1_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image1_t]]

// CHECK-DAG:             [[image2_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 2D 2 0 0 1 Unknown
// CHECK-DAG:          [[ra_image2_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image2_t]]
// CHECK-DAG:       [[ptr_u_image2_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image2_t]]
// CHECK-DAG:    [[ptr_u_ra_image2_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image2_t]]

// CHECK-DAG:             [[image3_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 3D 2 0 0 1 Unknown
// CHECK-DAG:          [[ra_image3_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image3_t]]
// CHECK-DAG:       [[ptr_u_image3_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image3_t]]
// CHECK-DAG:    [[ptr_u_ra_image3_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image3_t]]

// CHECK-DAG:             [[imageC_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float Cube 2 0 0 1 Unknown
// CHECK-DAG:          [[ra_imageC_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[imageC_t]]
// CHECK-DAG:       [[ptr_u_imageC_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[imageC_t]]
// CHECK-DAG:    [[ptr_u_ra_imageC_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_imageC_t]]

// CHECK-DAG:            [[image2A_t:%[_a-zA-Z0-9]+]] = OpTypeImage %float 2D 2 1 0 1 Unknown
// CHECK-DAG:         [[ra_image2A_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[image2A_t]]
// CHECK-DAG:      [[ptr_u_image2A_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[image2A_t]]
// CHECK-DAG:   [[ptr_u_ra_image2A_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_image2A_t]]

// CHECK-DAG:  [[sampled_image1_t:%[_a-zA-Z0-9]+]] = OpTypeSampledImage [[image1_t]]
// CHECK-DAG:  [[sampled_image2_t:%[_a-zA-Z0-9]+]] = OpTypeSampledImage [[image2_t]]
// CHECK-DAG:  [[sampled_image3_t:%[_a-zA-Z0-9]+]] = OpTypeSampledImage [[image3_t]]
// CHECK-DAG:  [[sampled_imageC_t:%[_a-zA-Z0-9]+]] = OpTypeSampledImage [[imageC_t]]
// CHECK-DAG: [[sampled_image2A_t:%[_a-zA-Z0-9]+]] = OpTypeSampledImage [[image2A_t]]

// CHECK-DAG:   [[sampler_heap:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_sampler_t]] UniformConstant
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] DescriptorSet 0
// CHECK-DAG:                                     OpDecorate [[sampler_heap]] Binding 1

// CHECK-DAG:   [[resource_heap_1d:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_image1_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_1d]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_1d]] Binding 0
// CHECK-DAG:   [[resource_heap_2d:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_image2_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_2d]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_2d]] Binding 0
// CHECK-DAG:   [[resource_heap_3d:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_image3_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_3d]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_3d]] Binding 0
// CHECK-DAG: [[resource_heap_cube:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_imageC_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_cube]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_cube]] Binding 0
// CHECK-DAG:   [[resource_heap_2A:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_image2A_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_2A]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_2A]] Binding 0

float4 main() : SV_Target {
  SamplerState Sampler = SamplerDescriptorHeap[0];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_sampler_t]] [[sampler_heap]] %uint_0
// CHECK: [[sampler:%[0-9]+]] = OpLoad [[sampler_t]] [[ptr]]

  float4 output = float(0.f).rrrr;

  Texture1D Texture1 = ResourceDescriptorHeap[1];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_image1_t]] [[resource_heap_1d]] %uint_1
// CHECK: [[texture:%[0-9]+]] = OpLoad [[image1_t]] [[ptr]]
  output += Texture1.Sample(Sampler, float(0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage [[sampled_image1_t]] [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] %float_0 None

  Texture2D Texture2 = ResourceDescriptorHeap[2];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_image2_t]] [[resource_heap_2d]] %uint_2
// CHECK: [[texture:%[0-9]+]] = OpLoad [[image2_t]] [[ptr]]
  output += Texture2.Sample(Sampler, float2(0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage [[sampled_image2_t]] [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_00]] None

  Texture3D Texture3 = ResourceDescriptorHeap[3];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_image3_t]] [[resource_heap_3d]] %uint_3
// CHECK: [[texture:%[0-9]+]] = OpLoad [[image3_t]] [[ptr]]
  output += Texture3.Sample(Sampler, float3(0, 0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage [[sampled_image3_t]] [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_000]] None

  TextureCube TextureC = ResourceDescriptorHeap[4];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_imageC_t]] [[resource_heap_cube]] %uint_4
// CHECK: [[texture:%[0-9]+]] = OpLoad [[imageC_t]] [[ptr]]
  output += TextureC.Sample(Sampler, float3(0, 0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage [[sampled_imageC_t]] [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_000]] None

  Texture2DArray Texture2A = ResourceDescriptorHeap[5];
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_image2A_t]] [[resource_heap_2A]] %uint_5
// CHECK: [[texture:%[0-9]+]] = OpLoad [[image2A_t]] [[ptr]]
  output += Texture2A.Sample(Sampler, float3(0, 0, 0));
// CHECK: [[sampled_image:%[0-9]+]] = OpSampledImage [[sampled_image2A_t]] [[texture]] [[sampler]]
// CHECK:                             OpImageSampleImplicitLod %v4float [[sampled_image]] [[float_000]] None

  return output;
}

