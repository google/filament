// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s

// Make sure there's no bit piece debug expression when
// the element covers the whole variable.

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-NOT: !DIExpression(DW_OP_bit_piece

float main(float a : A, float b : B) : SV_Target {
  float s[1] = {
    a+b,
  };

  [unroll]
  for (int i = 0; i < 1; i++)
    s[i] *= s[i];

  float result = 0;

  [unroll]
  for (int i = 0; i < 1; i++)
    result += s[i];

  return result;
}

