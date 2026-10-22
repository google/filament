// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[cu12:%[0-9]+]] = OpConstantComposite %v2uint %uint_1 %uint_2
// CHECK: [[cu123:%[0-9]+]] = OpConstantComposite %v3uint %uint_1 %uint_2 %uint_3

Texture2DMS      <int3>   t1;
Texture2DMSArray <float4> t2;

void main() {
  uint  pos  = uint(1);
  uint2 pos2 = uint2(1,2);

// CHECK:      [[pos2:%[0-9]+]] = OpLoad %v2uint %pos2
// CHECK-NEXT:   [[t1:%[0-9]+]] = OpLoad %type_2d_image %t1
// CHECK-NEXT:   [[f1:%[0-9]+]] = OpImageFetch %v4int [[t1]] [[pos2]] Sample %uint_7
// CHECK-NEXT: [[val1:%[0-9]+]] = OpVectorShuffle %v3int [[f1]] [[f1]] 0 1 2
// CHECK-NEXT:                 OpStore %a1 [[val1]]
  int3   a1 = t1.sample[7][pos2];

// CHECK-NEXT: [[pos:%[0-9]+]] = OpLoad %uint %pos
// CHECK-NEXT:  [[t2:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:  [[f2:%[0-9]+]] = OpImageFetch %v4float [[t2]] [[cu123]] Sample [[pos]]
// CHECK-NEXT: OpStore %a2 [[f2]]
  float4 a2 = t2.sample[pos][uint3(1,2,3)];
}
