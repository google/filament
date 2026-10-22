// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

void main() {
// CHECK: OpStore %pos1 %uint_1

  uint pos1 = 1;

// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_2
// CHECK: OpStore %a1 [[fetch_result]]
  float4 a1 = tex1d.mips[2][pos1];

}
