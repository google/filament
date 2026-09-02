// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[cu12:%[0-9]+]] = OpConstantComposite %v2uint %uint_1 %uint_2

// CHECK: [[type_2d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 0 1 Unknown
// CHECK: [[type_2d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image]]
// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2D<float4> tex2d;
vk::SampledTexture2DArray<float4> tex2dArray;

void main() {
// CHECK:                    OpStore %pos1 [[cu12]]
// CHECK-NEXT:    [[pos1:%[0-9]+]] = OpLoad %v2uint %pos1
// CHECK-NEXT: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_2d_image]] [[tex1_load]]
// CHECK-NEXT:      [[result1:%[0-9]+]] = OpImageFetch %v4float [[tex_img]] [[pos1]] Lod %uint_0
// CHECK-NEXT:                    OpStore %a1 [[result1]]
  uint2 pos1 = uint2(1,2);
  float4 a1 = tex2d[pos1];

// CHECK:    [[pos2:%[0-9]+]] = OpLoad %v3uint %pos2
// CHECK-NEXT: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArray
// CHECK-NEXT:    [[tex2_img:%[0-9]+]] = OpImage [[type_2d_image_array]] [[tex2_load]]
// CHECK-NEXT:      [[result2:%[0-9]+]] = OpImageFetch %v4float [[tex2_img]] [[pos2]] Lod %uint_0
// CHECK-NEXT:                    OpStore %a2 [[result2]]
  uint3 pos2 = uint3(1,2,3);
  float4 a2 = tex2dArray[pos2];
}
