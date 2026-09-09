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
// CHECK: i32 4)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 5)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 6)
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 7)

// CHECK: getelementptr
// CHECK: 0
// CHECK: store
// CHECK: getelementptr
// CHECK: 1
// CHECK: store
// CHECK: getelementptr
// CHECK: 2
// CHECK: store
// CHECK: getelementptr
// CHECK: 3
// CHECK: store
// CHECK: getelementptr
// CHECK: 4
// CHECK: store
// CHECK: getelementptr
// CHECK: 5
// CHECK: store
// CHECK: getelementptr
// CHECK: 6
// CHECK: store
// CHECK: getelementptr
// CHECK: 7
// CHECK: store

// CHECK: getelementptr
// CHECK: 8
// CHECK: store
// CHECK: getelementptr
// CHECK: 9
// CHECK: store
// CHECK: getelementptr
// CHECK: 10
// CHECK: store
// CHECK: getelementptr
// CHECK: 11
// CHECK: store
// CHECK: getelementptr
// CHECK: 12
// CHECK: store
// CHECK: getelementptr
// CHECK: 13
// CHECK: store
// CHECK: getelementptr
// CHECK: 14
// CHECK: store
// CHECK: getelementptr
// CHECK: 15
// CHECK: store



float4x4 Mat;
row_major float4x4 Mat2;
uint i;

float4 main() : SV_POSITION {
  float4x4 m = Mat + Mat2;
  return m[i];
}
