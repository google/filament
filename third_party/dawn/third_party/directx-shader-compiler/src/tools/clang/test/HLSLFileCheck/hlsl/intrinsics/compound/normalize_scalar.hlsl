// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: fmul
// CHECK: Rsqrt

float main(float a: A) : SV_TARGET
{
  return normalize(a);
}
