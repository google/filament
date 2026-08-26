// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// CHECK: @main

// i == 0
// CHECK: call float @dx.op.unary.f32(i32 12

// i == 1
// CHECK: call float @dx.op.unary.f32(i32 12
// CHECK: call float @dx.op.unary.f32(i32 12

// i == 2
// CHECK: call float @dx.op.unary.f32(i32 12
// CHECK: call float @dx.op.unary.f32(i32 12

// CHECK-NOT: call float @dx.op.unary.f32(i32 13

[RootSignature("")]
float main(float foo : FOO) : SV_Target {
  float result = 0;
  [unroll]
  for (uint i = 0; i < 3; i++) {
    [unroll]
    for (uint j = 0; j <= i; j++) {
      result += cos(j + i + foo);
    }
  }
  return result;
}

