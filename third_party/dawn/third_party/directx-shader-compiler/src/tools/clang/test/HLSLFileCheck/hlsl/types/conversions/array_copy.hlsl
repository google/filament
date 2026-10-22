// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure memcpy float3[1], float3[1][1] works.

// CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
// CHECK:fadd

struct A
{
  float3 a;
  float3 b[1];
};

float doubleDim(A a[1]) {
  return a[0].b[0].x + a[0].b[0].y;
}

float3 c;
float3 d[1];

A getA() {
  A a;
  a.a = c;
  a.b = d;
  return a;
}

float main() : SV_Target {
  A a[1];
  a[0] = getA();
  return doubleDim(a);
}