// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure default val for integer cbuffer element is ignored.
// CHECK: add

int c = 19;

float main() : SV_Target {
  int x = 1+c;
  return x;
}