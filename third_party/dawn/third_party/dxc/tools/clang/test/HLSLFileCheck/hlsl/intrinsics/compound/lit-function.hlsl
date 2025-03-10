// RUN: %dxc -T ps_6_0 -E main  %s | FileCheck %s

// Verify lit function defined as lit(ambient, diffuse, specular, 1) where:
// ambient = 1.
// diffuse = ((n l) < 0) ? 0 : n l.
// specular = ((n l) < 0) || ((n h) < 0) ? 0 : ((n h) ^ m).

// CHECK: fcmp
// CHECK: select
// CHECK: fcmp
// CHECK: or
// CHECK: Log
// CHECK: fmul
// CHECK: Exp
// CHECK: select

float4 main(float a : A, float b : B, float c : C) : SV_Target
{
  return lit(a, b, c);
}
