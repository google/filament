// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

void main() {
// CHECK: OpStore %pos1 [[pos1_init:%[a-zA-Z0-9_]+]]

  uint2 pos1 = uint2(1,2);
// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %v2uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image_array [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_2
// CHECK: OpStore %a1 [[fetch_result]]

  float4 a1 = tex1dArray.mips[2][pos1];

}
