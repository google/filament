// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: slt
// CHECK: discard
// CHECK: olt
// CHECK: discard

float main(float a : A, int b : B, float r : R) : SV_Target
{
  clip(b < 0? -1 : 1);

  clip(a);
  return r;
}
