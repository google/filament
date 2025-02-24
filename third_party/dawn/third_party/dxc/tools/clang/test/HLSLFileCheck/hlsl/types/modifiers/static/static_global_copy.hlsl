// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s



// Make sure initialize static global inside user function can still be propagated.
// CHECK-NOT: alloca

struct A {
  float4 x[25];
};

A a;

static A a2;

void set(A aa) {
   aa = a;
}

float4 main(uint l:L) : SV_Target {
  set(a2);
  return a2.x[l];
}
