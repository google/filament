// RUN: %dxc -T lib_6_4 -Wdouble-promotion -HV 2021 -verify %s
// RUN: %dxc -T lib_6_4 -Wdouble-promotion -HV 2021 -enable-16bit-types -verify %s
// RUN: %dxc -T lib_6_4 -Wdouble-promotion -HV 202x -verify %s
// RUN: %dxc -T lib_6_4 -Wdouble-promotion -HV 202x -enable-16bit-types -verify %s

void LitFloat() {
#if __HLSL_VERSION <= 2021
#ifndef __HLSL_ENABLE_16_BIT
  min16float m = 1.0;
#endif
  half h = 1.0;
  float f = 1.0;
  double d = 1.0;
#else
#ifndef __HLSL_ENABLE_16_BIT
  min16float m = 1.0;
#endif
  half h = 1.0;
  float f = 1.0;
  double d = 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
#endif
}

void HalfSuffix() {
#ifndef __HLSL_ENABLE_16_BIT
  min16float m = 1.0h;
#if __HLSL_VERSION <= 2021
  float f = 1.0h;
  double d = 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
#else
  float f = 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  double d = 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
#endif
#else
  float f = 1.0h;  // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  double d = 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
#endif
  half h = 1.0h;
}

void FloatSuffix() {
#ifndef __HLSL_ENABLE_16_BIT
  min16float m = 1.0f;
#endif
  half h = 1.0f;
  float f = 1.0f;
  double d = 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
}

void DoubleSuffix() {
#ifndef __HLSL_ENABLE_16_BIT
  min16float m = 1.0l;
#endif
  half h = 1.0l;
  float f = 1.0l;
  double d = 1.0l;
}

void TernaryFun(bool B) {
#if __HLSL_VERSION > 2021
#ifndef __HLSL_ENABLE_16_BIT
  min16float m0 = B ? 1.0 : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m1 = B ? 1.0 : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m2 = B ? 1.0h : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m3 = B ? 1.0 : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m4 = B ? 1.0f : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m7 = B ? 1.0h : 1.0h; // expected-warning{{conversion from larger type 'half' to smaller type 'min16float', possible loss of data}}
  min16float m9 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m10 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m13 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m14 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m15 = B ? 1.0f : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}

  half h0 = B ? 1.0 : 1.0;
  half h1 = B ? 1.0 : 1.0h;
  half h2 = B ? 1.0h : 1.0;
  half h3 = B ? 1.0 : 1.0f;
  half h4 = B ? 1.0f : 1.0;
  half h5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h7 = B ? 1.0h : 1.0h;
  half h9 = B ? 1.0h : 1.0f;
  half h10 = B ? 1.0f : 1.0h;
  half h11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h13 = B ? 1.0f : 1.0h;
  half h14 = B ? 1.0h : 1.0f;
  half h15 = B ? 1.0f : 1.0f;
  half h16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
#else
  half h0 = B ? 1.0 : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h1 = B ? 1.0 : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h2 = B ? 1.0h : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h3 = B ? 1.0 : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h4 = B ? 1.0f : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h7 = B ? 1.0h : 1.0h;
  half h9 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h10 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h13 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h14 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h15 = B ? 1.0f : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
#endif
  
  float f0 = B ? 1.0 : 1.0;
  float f1 = B ? 1.0 : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f2 = B ? 1.0h : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f3 = B ? 1.0 : 1.0f;
  float f4 = B ? 1.0f : 1.0;
  float f5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f7 = B ? 1.0h : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f9 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f10 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f12 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}} expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f13 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f14 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f15 = B ? 1.0f : 1.0f;
  float f16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}

  double d0 = B ? 1.0 : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d1 = B ? 1.0 : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d2 = B ? 1.0h : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d3 = B ? 1.0 : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d4 = B ? 1.0f : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d5 = B ? 1.0 : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d6 = B ? 1.0l : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d7 = B ? 1.0h : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d9 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d10 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d11 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d12 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d13 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d14 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d15 = B ? 1.0f : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d16 = B ? 1.0f : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d17 = B ? 1.0l : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d18 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d19 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d20 = B ? 1.0l : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d21 = B ? 1.0f : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d22 = B ? 1.0l : 1.0l;
#else // __HLSL_VERSION > 2021

#ifndef __HLSL_ENABLE_16_BIT
  min16float m0 = B ? 1.0 : 1.0;
  min16float m1 = B ? 1.0 : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m2 = B ? 1.0h : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m3 = B ? 1.0 : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m4 = B ? 1.0f : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m7 = B ? 1.0h : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m9 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m10 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m13 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m14 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m15 = B ? 1.0f : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'min16float', possible loss of data}}
  min16float m16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}
  min16float m22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'min16float', possible loss of data}}

  half h0 = B ? 1.0 : 1.0;
  half h1 = B ? 1.0 : 1.0h;
  half h2 = B ? 1.0h : 1.0;
  half h3 = B ? 1.0 : 1.0f;
  half h4 = B ? 1.0f : 1.0;
  half h5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h7 = B ? 1.0h : 1.0h;
  half h9 = B ? 1.0h : 1.0f;
  half h10 = B ? 1.0f : 1.0h;
  half h11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h13 = B ? 1.0f : 1.0h;
  half h14 = B ? 1.0h : 1.0f;
  half h15 = B ? 1.0f : 1.0f;
  half h16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}
  half h22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half', possible loss of data}}

  float f1 = B ? 1.0 : 1.0h;
  float f2 = B ? 1.0h : 1.0;
  float f9 = B ? 1.0h : 1.0f;
  float f10 = B ? 1.0f : 1.0h;
  float f13 = B ? 1.0f : 1.0h;
  float f14 = B ? 1.0h : 1.0f;
  float f7 = B ? 1.0h : 1.0h;
  float f11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}

  double d1 = B ? 1.0 : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d2 = B ? 1.0h : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d7 = B ? 1.0h : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d9 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d10 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d11 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d12 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d13 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d14 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d18 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d19 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
#else
  half h0 = B ? 1.0 : 1.0;
  half h1 = B ? 1.0 : 1.0h;
  half h2 = B ? 1.0h : 1.0;
  half h3 = B ? 1.0 : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h4 = B ? 1.0f : 1.0; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h7 = B ? 1.0h : 1.0h;
  half h9 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h10 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h13 = B ? 1.0f : 1.0h; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h14 = B ? 1.0h : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h15 = B ? 1.0f : 1.0f; // expected-warning{{conversion from larger type 'float' to smaller type 'half'}}
  half h16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}
  half h22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'half'}}

  float f1 = B ? 1.0 : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f2 = B ? 1.0h : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f9 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f10 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f13 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f14 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f7 = B ? 1.0h : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f11 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f12 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f18 = B ? 1.0l : 1.0h; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}
  float f19 = B ? 1.0h : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'float'}}

  double d1 = B ? 1.0 : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d2 = B ? 1.0h : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d7 = B ? 1.0h : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d9 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d10 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d11 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d12 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d13 = B ? 1.0f : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d14 = B ? 1.0h : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d18 = B ? 1.0l : 1.0h; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
  double d19 = B ? 1.0h : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'half' to 'double'}}
#endif
  
  float f0 = B ? 1.0 : 1.0;
  float f3 = B ? 1.0 : 1.0f;
  float f4 = B ? 1.0f : 1.0;
  float f5 = B ? 1.0 : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f6 = B ? 1.0l : 1.0; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f15 = B ? 1.0f : 1.0f;
  float f16 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f17 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f20 = B ? 1.0l : 1.0f; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f21 = B ? 1.0f : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  float f22 = B ? 1.0l : 1.0l; // expected-warning{{conversion from larger type 'double' to smaller type 'float', possible loss of data}}
  
  double d0 = B ? 1.0 : 1.0;
  double d3 = B ? 1.0 : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d4 = B ? 1.0f : 1.0; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d5 = B ? 1.0 : 1.0l;
  double d6 = B ? 1.0l : 1.0;
  double d15 = B ? 1.0f : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}} expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d16 = B ? 1.0f : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d17 = B ? 1.0l : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d20 = B ? 1.0l : 1.0f; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d21 = B ? 1.0f : 1.0l; // expected-warning{{implicit conversion increases floating-point precision: 'float' to 'double'}}
  double d22 = B ? 1.0l : 1.0l;

#endif // __HLSL_VERSION > 2021
}
