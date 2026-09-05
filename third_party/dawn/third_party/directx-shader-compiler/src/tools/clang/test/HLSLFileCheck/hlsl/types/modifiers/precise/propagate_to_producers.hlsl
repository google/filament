// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise applies to instructions producing the value.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  float square = f * f;
  precise float result = square;
  return result;
}