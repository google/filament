// RUN: %dxc -Od -E main -T ps_6_0 -HV 2018 %s | FileCheck %s
// CHECK: @main

// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK-NOT: @dx.op.unary.f32(i32 13

// Confirm that loops with fairly complex exit conditions
// should be able to be unrolled

[RootSignature("")]
float main(float y : Y) : SV_Target {
  float x = 0;

  static const uint kLoopCount = 512;

  int j = 10;
  [unroll]
  for (uint i = 0; i < kLoopCount && j > 2; i += 16) {
    x = sin(x * x + y);
    i -= 8;
    j -= 1;
  }
  return x;
}
