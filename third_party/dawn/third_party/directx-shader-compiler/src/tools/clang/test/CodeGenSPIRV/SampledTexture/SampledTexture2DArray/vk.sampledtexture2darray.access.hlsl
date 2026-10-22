// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[cu12:%[0-9]+]] = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_0

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

void main() {
// CHECK:                    OpStore %pos1 [[cu12]]
// CHECK-NEXT:    [[pos1:%[0-9]+]] = OpLoad %v3uint %pos1
// CHECK-NEXT: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_2d_image_array]] [[tex1_load]]
// CHECK-NEXT:      [[result1:%[0-9]+]] = OpImageFetch %v4float [[tex_img]] [[pos1]] Lod %uint_0
// CHECK-NEXT:                    OpStore %a1 [[result1]]
  uint3 pos1 = uint3(1,2,0);
  float4 a1 = tex2darray[pos1];

}
