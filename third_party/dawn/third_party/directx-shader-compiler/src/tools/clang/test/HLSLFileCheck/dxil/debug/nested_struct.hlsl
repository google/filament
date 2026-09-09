// RUN: %dxc -Zi -E main -Od -T ps_6_0 %s | FileCheck %s

// Make sure all elements of the struct (even when there are nested structs)
// are at distinct offsets.

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 64, 32)

struct K {
  float foo;
};

struct S {
  float foo;
  K bar;
  float baz;
};

float main(float a : A, float b : B) : SV_Target {
  S s;
  s.foo = a;
  s.bar.foo = a+b;
  s.baz = a+b;
  return s.bar.foo + s.foo + s.baz;
}

