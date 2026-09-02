// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise modifier on a matrix has an effect.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float2x2 main(float2x2 m : IN) : OUT
{
  precise float2x2 result = m * m;
  return result;
}
