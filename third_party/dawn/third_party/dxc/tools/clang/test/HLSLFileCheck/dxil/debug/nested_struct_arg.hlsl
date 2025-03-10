// RUN: %dxc -Zi -E main -Od -T ps_6_0 %s | FileCheck %s

// Make sure all elements of the struct in an arg (even when there are nested
// structs) are at distinct offsets.

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: DW_OP_bit_piece

struct K_ARG {
  float foo : KFOO;
};

struct S_ARG {
  float foo : FOO;
  K_ARG bar;
  float baz : BAZ;
};

float main(S_ARG s) : SV_Target {
  return s.bar.foo + s.foo + s.baz;
}

