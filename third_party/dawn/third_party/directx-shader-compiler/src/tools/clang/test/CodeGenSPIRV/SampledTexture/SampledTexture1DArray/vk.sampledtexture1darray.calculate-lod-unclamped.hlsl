// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

void main() {
// CHECK: OpStore %xy [[xy_init:%[a-zA-Z0-9_]+]]

  float2 xy = float2(0.5, 0.5);

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[xy:%[a-zA-Z0-9_]+]] = OpLoad %v2float %xy
// CHECK: [[coord:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[xy]] 0
// CHECK: [[lod_query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex1_load]] [[coord]]
// CHECK: [[unclamped_lod:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[lod_query]] 1
// CHECK: OpStore %lod1 [[unclamped_lod]]

  float lod1 = tex1dArray.CalculateLevelOfDetailUnclamped(xy);

}
