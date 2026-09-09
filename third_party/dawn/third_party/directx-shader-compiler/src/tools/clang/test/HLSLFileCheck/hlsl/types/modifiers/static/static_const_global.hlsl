// RUN: %dxilver 1.5 | %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// Make sure ST is removed
// CHECK-NOT: @ST

cbuffer A {
  float a;
  int b;
}

const static struct {
  float a;
  int b;
}  ST = { a, b };

float4 test() {
  return ST.a + ST.b;
}

float test2() {
  return ST.a - ST.b;
}
