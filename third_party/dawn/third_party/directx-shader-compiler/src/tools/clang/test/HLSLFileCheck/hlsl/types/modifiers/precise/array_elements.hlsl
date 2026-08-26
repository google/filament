// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise modifier on an array has an effect.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  precise float a[1] = { f * f };
  return a[0];
}