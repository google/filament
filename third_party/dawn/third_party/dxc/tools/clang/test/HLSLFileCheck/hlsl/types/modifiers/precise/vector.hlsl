// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise modifier on a vector has an effect.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float2 main(float2 v : IN) : OUT
{
  precise float2 result = v * v;
  return result;
}