// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_3d_image = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_3d_image

vk::SampledTexture3D<float4> tex3d;

void main() {
  uint3 pos1 = uint3(1, 2, 3);

// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %v3uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex3d
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_3d_image [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_2
// CHECK: OpStore %a1 [[fetch_result]]
  float4 a1 = tex3d.mips[2][pos1];
}
