// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 0)
// CHECK: extractvalue
// CHECK: 2
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 1)
// CHECK: extractvalue
// CHECK: 2
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 2)
// CHECK: extractvalue
// CHECK: 2
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 3)
// CHECK: extractvalue
// CHECK: 2
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 6)
// CHECK: extractvalue
// CHECK: 1
// CHECK: extractvalue
// CHECK: 1
// CHECK: extractvalue
// CHECK: 2
// CHECK: extractvalue
// CHECK: 3

float4x4 Mat;
row_major float4x4 Mat2;

float4 main() : SV_POSITION {
  float4x4 m = Mat + Mat2;
  return m[2];
}
