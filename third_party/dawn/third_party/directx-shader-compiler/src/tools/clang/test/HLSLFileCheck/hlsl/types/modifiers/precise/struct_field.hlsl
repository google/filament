// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise modifier on a struct field has an effect.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float main(float f : IN) : OUT
{
  struct { precise float f; } s = { f * f };
  return s.f;
}