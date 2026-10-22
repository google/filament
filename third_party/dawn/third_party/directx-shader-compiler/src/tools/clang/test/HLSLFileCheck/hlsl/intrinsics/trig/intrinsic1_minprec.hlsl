// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Shader extensions for 11.1

// CHECK: Sin
// CHECK: Hsin
// CHECK: Cos
// CHECK: Hcos
// CHECK: Tan
// CHECK: Htan
// CHECK: Asin
// CHECK: Acos
// CHECK: Atan
// CHECK: FirstbitLo
// CHECK: Countbits
// CHECK: Bfrev
// CHECK: Rsqrt
// CHECK: Bfi
// CHECK: Msad

min16float v;
uint i;

uint s;
uint2 a;
uint4 d;

float4 main(float4 arg : A) : SV_TARGET {
  min16float t = sin(v) + sinh(v) + cos(v) + cosh(v) + tan(v) + tanh(v);
  t = asin(t) + acos(t) + atan(t) + atan2(v,3);
  t += firstbitlow(i) + countbits(i) + reversebits(i);
  t = rsqrt(t);
  float4 si,co;
  sincos(arg, si, co);
  return t+ msad4(s,a,d) + si + co;
}
