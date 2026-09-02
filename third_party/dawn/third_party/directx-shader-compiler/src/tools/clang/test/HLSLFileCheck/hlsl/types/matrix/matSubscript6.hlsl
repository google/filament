// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Mat[i][2] is col major. Base address 0.
// 2 is for [2], i is indexing, so need all 4 component.
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 2)
// CHECK: extractvalue
// CHECK: 0
// CHECK: extractvalue
// CHECK: 1
// CHECK: extractvalue
// CHECK: 2
// CHECK: extractvalue
// CHECK: 3

// Mat2[1][2] is row major. Base address is 16.
// 16 bytes -> 4 dwords then add 1 -> 5.
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 5)
// CHECK: extractvalue
// CHECK: 2

float4x4 Mat;
row_major float4x4 Mat2;
uint i;

float4 main() : SV_POSITION {
  return Mat[i][2] + Mat2[1][2];
}
