// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// Make sure mad is not optimized when has precise.
// CHECK: dx.op.tertiary.f32

float main(float a : A, float b :B) : SV_Target {
  precise float t = mad(a, 0, b);
  return t;
}