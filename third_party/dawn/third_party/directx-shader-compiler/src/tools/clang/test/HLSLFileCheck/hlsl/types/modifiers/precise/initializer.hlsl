// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise applies to the initializer of the variable.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  precise float result = f * f;
  return result;
}