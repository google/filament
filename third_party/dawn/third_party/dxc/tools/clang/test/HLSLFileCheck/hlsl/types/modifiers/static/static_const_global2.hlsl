// RUN: %dxc -T ps_6_1 -E main -fcgl %s | FileCheck %s

// Make sure ST only used once for decl.
// CHECK: @ST
// CHECK-NOT: @ST

cbuffer A {
  float a;
  int b;
}

const static struct {
  float a;
  int b;
}  ST = { a, b };

float4 main() : SV_TARGET  {
  return ST.a + ST.b;
}
