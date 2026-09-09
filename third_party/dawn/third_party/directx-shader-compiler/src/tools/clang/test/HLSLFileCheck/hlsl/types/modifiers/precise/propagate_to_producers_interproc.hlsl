// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise applies retroactively to instructions
// producing a given value, interprocedurally.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float square(float f) { return f * f; }

float make_precise(float f)
{
  precise float pf = f;
  return f;
}

float main(float f : IN) : OUT
{
  float result = square(f);
  return make_precise(result);
}