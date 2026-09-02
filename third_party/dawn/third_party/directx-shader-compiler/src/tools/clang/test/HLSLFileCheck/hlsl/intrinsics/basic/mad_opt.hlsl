// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure no intrinsic for mad.
// CHECK-NOT: dx.op.tertiary


// Make sure a, c, e are not used.
// CHECK-NOT: dx.op.loadInput.f32(i32 4, i32 0
// CHECK-NOT: dx.op.loadInput.i32(i32 4, i32 2
// CHECK-NOT: dx.op.loadInput.i32(i32 4, i32 4

// Make sure b, d, f are used.
// CHECK: dx.op.loadInput.i32(i32 4, i32 5
// CHECK: dx.op.loadInput.i32(i32 4, i32 3
// CHECK: dx.op.loadInput.f32(i32 4, i32 1

// CHECK: fadd fast
// CHECK: fadd fast

float main(float a : A, float b :B, int c : C, int d :D, uint e :E, uint f :F) : SV_Target {
  return mad(a, 0, b) + mad(0, c, d) + mad(e, 0, f);
}