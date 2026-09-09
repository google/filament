// RUN: %dxc -E main -T ps_6_0 -Gec -Zi %s | FileCheck %s

// Make sure debug info for s.

// CHECK:!DIGlobalVariable(name: "s.0"
// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}


struct S {
  float a;
};

cbuffer A {
  S s;
};



float main() : SV_Target {

  s.a++;
  return s.a;
}