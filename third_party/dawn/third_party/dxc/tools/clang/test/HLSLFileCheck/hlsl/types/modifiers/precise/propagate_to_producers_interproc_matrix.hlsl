// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that precise applies retroactively to matrix instructions
// producing a given value, interprocedurally.

// CHECK-NOT: fmul fast float
// CHECK: fmul float
// CHECK-NOT: fmul fast float

float3x3 square(float3x3 f) { return f * f; }

float3x3 make_precise(float3x3 f)
{
  precise float3x3 pf = f;
  return f;
}

float3x3 main(float3x3 f : IN) : OUT
{
  float3x3 result = square(f);
  return make_precise(result);
}