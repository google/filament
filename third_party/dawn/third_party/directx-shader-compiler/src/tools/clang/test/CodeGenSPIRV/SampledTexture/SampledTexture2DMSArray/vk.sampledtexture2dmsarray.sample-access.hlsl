// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[type_2d_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms_array]]

vk::SampledTexture2DMSArray<float4> tex2dMSArray;

void main() {
  uint2 pos2 = uint2(1,2);

// CHECK:      [[pos3:%[0-9]+]] = OpLoad %v3uint %pos3
// CHECK-NEXT: [[tex2:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT: [[img2:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[tex2]]
// CHECK-NEXT: [[f2:%[0-9]+]] = OpImageFetch %v4float [[img2]] [[pos3]] Sample %uint_4
// CHECK-NEXT: OpStore %a [[f2]]
  uint3 pos3 = uint3(1,2,3);
  float4 a = tex2dMSArray.sample[4][pos3];
}
