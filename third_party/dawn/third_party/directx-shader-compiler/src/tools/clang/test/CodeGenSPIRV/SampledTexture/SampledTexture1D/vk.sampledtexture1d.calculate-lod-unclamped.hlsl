// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

void main() {
// CHECK: OpStore %x %float_0_5
  float x = 0.5;

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[x:%[a-zA-Z0-9_]+]] = OpLoad %float %x
// CHECK: [[lod_query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex1_load]] [[x]]
// CHECK: [[unclamped_lod:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[lod_query]] 1
// CHECK: OpStore %lod1 [[unclamped_lod]]
  float lod1 = tex1d.CalculateLevelOfDetailUnclamped(x);

}
