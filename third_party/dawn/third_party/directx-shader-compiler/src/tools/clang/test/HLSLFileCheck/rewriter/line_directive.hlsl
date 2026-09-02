// RUN: %dxr -Emain -remove-unused-globals  -line-directive %s | FileCheck %s

#include "header.h"

StructuredBuffer<C> buf : register(t0, space6);

float foo() {
  return 3;
}

float main(uint i:I) : SV_Target {
  float m = 2;
  float n = sin(i);  return buf[i].c * m + n;
}

// CHECK:#line 1 {{.*}}line_directive.hlsl
// CHECK:#line 1 {{.*}}header.h
// CHECK-NEXT:struct A
// CHECK-NOT:struct X
// CHECK-NOT:bbb
// CHECK:#line 16 {{.*}}header.h
// CHECK-NEXT:struct C
// CHECK-NOT:struct D
// CHECK:#line 5 {{.*}}line_directive.hlsl
// CHECK-NEXT:StructuredBuffer<C> buf
// CHECK-NOT:foo
// CHECK:#line 11 {{.*}}line_directive.hlsl
// CHECK-NEXT:float main