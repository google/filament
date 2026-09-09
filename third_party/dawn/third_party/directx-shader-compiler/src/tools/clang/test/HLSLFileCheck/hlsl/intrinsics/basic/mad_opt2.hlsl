// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// Make sure no intrinsic for mad.
// CHECK-NOT: dx.op.tertiary
// Make sure have 3 fast float add and 2 int add.
// CHECK: add i32
// CHECK: add i32
// CHECK: fadd fast
// CHECK: fadd fast
// CHECK: fadd fast


float main(float a : A, float b :B, int c : C, int d :D, uint e :E, uint f :F) : SV_Target {
  return mad(a, 1, b) + mad(1, c, d) + mad(e, 1, f);
}