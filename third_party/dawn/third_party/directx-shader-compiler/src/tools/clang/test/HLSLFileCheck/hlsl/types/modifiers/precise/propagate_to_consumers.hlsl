// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s | XFail GitHub #2079

// Test that precise propagates to operations consuming the value.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  precise float pf = f;
  return pf * pf;
}