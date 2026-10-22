// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure initialize static global inside user function can still be propagated.
// CHECK-NOT: alloca

// Make sure cbuffer is used.
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoad

// Make sure use of zero initializer get zero.
// CHECK: call void @dx.op.storeOutput.f32(i32 5, i32 0, i32 0, i8 1, float 0.000000e+00)

struct A {
  float4 x[25];
};

A a;

static A a2;

void set(out A aa) {
   aa = a;
}

float2 main(uint l:L) : SV_Target {
  float m = a2.x[l].x;
  set(a2);
  return float2(a2.x[l].x,m);
}
