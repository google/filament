// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure default val for cbuffer element is ignored.
// CHECK: fadd

float c = 0.91;

float main() : SV_Target {
  const float x = 1+c;
  return x;
}