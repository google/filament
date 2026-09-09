// RUN: %dxc -Od -E main -T ps_6_0 %s | FileCheck %s
// CHECK: @main
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13
// CHECK: @dx.op.unary.f32(i32 13

// CHECK-NOT: @dx.op.unary.f32(i32 13

// Confirm that loops with greater than 1 step should be able to be unrolled

[RootSignature("")]
float main(float y : Y) : SV_Target {
  float x = 0;

  static const uint kLoopCount = 512;

  [unroll]
  for (uint i = 0; i < kLoopCount; i += 32) {
    x = sin(x * x + y);
  }
  return x;
}
