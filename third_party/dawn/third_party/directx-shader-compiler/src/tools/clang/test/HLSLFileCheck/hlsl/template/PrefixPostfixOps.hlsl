// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s -HV 2021 -DCHECK_DIAGNOSTICS | FileCheck -check-prefix=DIAG %s

template<typename T>
T preinc(T t0) {
  T r = ++t0;
  return r;
}

template<typename T>
T postinc(T t0) {
  T r = t0++;
  return r;
}

template<typename T>
T predec(T t0) {
  T r = --t0;
  return r;
}

template<typename T>
T postdec(T t0) {
  T r = t0--;
  return r;
}

struct S {
  int4 i4;
  float2 f2;
  float3x3 f3x3;
  bool4 b4;
};

void main() : SV_Target {
  int i;
  int1 i1;
  int2 i2;
  int3 i3;
  int4 i4;
  unsigned int j;
  unsigned int2x2 j2x2;
  float x;
  float3 x3;
  bool b;
  bool2 b2;
  S s;
  float4x3 f4arr[11];

#ifndef CHECK_DIAGNOSTICS

// CHECK:define void @main

  preinc(i);
  preinc(i1);
  preinc(i2);
  preinc(i3);
  preinc(i4);
  preinc(i4.x);
  preinc(i4.yz);
  preinc(i4.xyz);
  preinc(j);
  preinc(j2x2);
  preinc(x);
  preinc(x3);
  preinc(s.i4);
  preinc(s.f2);
  preinc(s.f3x3);
  preinc(f4arr[9]);
  preinc(f4arr[9][2]);
  preinc(f4arr[3][1].xz);

  postinc(i);
  postinc(i1);
  postinc(i2);
  postinc(i3);
  postinc(i4);
  postinc(i4.x);
  postinc(i4.yz);
  postinc(i4.xyz);
  postinc(j);
  postinc(j2x2);
  postinc(x);
  postinc(x3);
  postinc(s.i4);
  postinc(s.f2);
  postinc(s.f3x3);
  postinc(f4arr[9]);
  postinc(f4arr[5][2]);
  postinc(f4arr[3][1].xz);

  predec(i);
  predec(i1);
  predec(i2);
  predec(i3);
  predec(i4);
  predec(i4.x);
  predec(i4.yz);
  predec(i4.xyz);
  predec(j);
  predec(j2x2);
  predec(x);
  predec(x3);
  predec(s.i4);
  predec(s.f2);
  predec(s.f3x3);
  postinc(f4arr[9]);
  postinc(f4arr[4][3]);
  postinc(f4arr[9][2].yz);

  postdec(i);
  postdec(i1);
  postdec(i2);
  postdec(i3);
  postdec(i4);
  postdec(i4.x);
  postdec(i4.yz);
  postdec(i4.xyz);
  postdec(j);
  postdec(j2x2);
  postdec(x);
  postdec(x3);
  postdec(s.i4);
  postdec(s.f2);
  postdec(s.f3x3);
  postdec(f4arr[9]);
  postdec(f4arr[4][3]);
  postdec(f4arr[9][2].yz);

#else

  // DIAG-NOT: define void @main

  // DIAG: function cannot return array type
  preinc(f4arr);

  // DIAG: error: scalar, vector, or matrix expected
  preinc(s);

  // DIAG: error: operator cannot be used with a bool lvalue
  // DIAG: error: operator cannot be used with a bool lvalue
  // DIAG: error: operator cannot be used with a bool lvalue
  preinc(b);
  preinc(b2);
  preinc(s.b4);

  // DIAG-NOT: error

#endif

}
