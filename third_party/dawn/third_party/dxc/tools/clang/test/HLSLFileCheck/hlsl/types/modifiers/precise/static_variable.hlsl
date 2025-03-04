// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s | XFail GitHub #2078

// Test that the precise modifier applies to static variables.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

static precise float g;
float main(float f : IN) : OUT
{
  g = f * f;
  return g;
}