// RUN: %dxc -Zi -E main -T ps_6_0 %s | FileCheck %s

// Make sure the bit pieces have the offset in bits

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 96, 32)

float main(float a : A, float b : B, float c : C, float d : D, float e : E) : SV_Target {
  float s[4] = {
    a+b,
    b+c,
    c+d,
    d+e,
  };

  [unroll]
  for (int i = 0; i < 4; i++)
    s[i] *= s[i];

  float result = 0;

  [unroll]
  for (int i = 0; i < 4; i++)
    result += s[i];

  return result;
}

