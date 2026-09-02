// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure default val for cbuffer element is ignored.

// CHECK-NOT: float 5.000000e-01
// CHECK: cbufferLoad
// CHECK: Sqrt

float t = 0.5;

float main() : SV_TARGET
{
  return sqrt(t);
}