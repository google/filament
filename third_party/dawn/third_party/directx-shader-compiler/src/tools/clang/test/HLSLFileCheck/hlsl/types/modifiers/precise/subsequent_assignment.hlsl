// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise applies to the value producers
// after the initial assignment.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  precise float result = 0;
  result = f * f;
  return result;
}