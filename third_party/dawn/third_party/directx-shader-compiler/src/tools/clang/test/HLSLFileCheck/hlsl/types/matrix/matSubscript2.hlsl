// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 0)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 1)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 2)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 3)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 6)

float4x4 Mat;
row_major float4x4 Mat2;
uint i;

float4 main() : SV_POSITION {
  return Mat[2] + Mat2[2];
}
