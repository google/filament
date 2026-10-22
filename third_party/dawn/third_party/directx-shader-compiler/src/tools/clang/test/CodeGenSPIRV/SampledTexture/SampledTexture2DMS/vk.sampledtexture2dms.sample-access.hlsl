// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[type_2d_image_ms:%[a-zA-Z0-9_]+]] = OpTypeImage %int 2D 0 0 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms]]

vk::SampledTexture2DMS<int3> tex2dMS;

void main() {
  uint2 pos2 = uint2(1,2);

// CHECK:      [[pos2:%[0-9]+]] = OpLoad %v2uint %pos2
// CHECK-NEXT: [[tex1:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT: [[img1:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[tex1]]
// CHECK-NEXT: [[f1:%[0-9]+]] = OpImageFetch %v4int [[img1]] [[pos2]] Sample %uint_3
// CHECK-NEXT: [[val1:%[0-9]+]] = OpVectorShuffle %v3int [[f1]] [[f1]] 0 1 2
// CHECK-NEXT:                 OpStore %a1 [[val1]]
  int3 a1 = tex2dMS.sample[3][pos2];
}
