// RUN: %dxc -T lib_6_3 -validator-version 1.3 -auto-binding-space 11 %s | FileCheck %s

// CHECK-NOT: llvm.global_ctors
// CHECK: !llvm.ident

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
