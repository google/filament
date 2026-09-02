// RUN: %dxc -E main -T ps_6_0 -HV 2021 -DNOVEC %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG


template<typename T>
T ternary_and(T t0, T t1) {
  return t0 && t1 ? t0 : t1;
}

template<typename T>
T ternary_or(T t0, T t1) {
  return t0 || t1 ? t0 : t1;
}

typedef struct {
  int4 a, b;
} S;


void main() : SV_Target {
  bool ba, bb;
  bool3 b3a, b3b;
  bool4x4 b4x4a, b4x4b;
  int ia, ib;
  int3 i3a, i3b;
  uint ua, ub;
  uint2 u2a, u2b;
  uint4 u4a, u4b;
  float fa, fb;
  float4 f4a, f4b;
  int iarra[8], iarrb[8];
  S sa, sb;

#ifdef CHECK_DIAGNOSTICS

  // DIAG-NOT: define void @main

  // DIAG: deduced conflicting types for parameter
  ternary_or(bb, sa);
  // DIAG: function cannot return array type
  ternary_or(iarra, iarrb);

  // DIAG: error: scalar, vector, or matrix expected
  ternary_and(sa, sb);
  // DIAG: error: scalar, vector, or matrix expected
  ternary_or(sa, sb);


#else

// CHECK: define void @main

  ternary_and(ba, bb);
  ternary_and(ia, ib);
  ternary_and(ua, ub);
  ternary_and(fa, fb);
  ternary_and(int(fa), ib);
  ternary_or(iarra[2], iarra[4]);

#ifndef NOVEC
  ternary_and(b3a, b3b);
  ternary_and(b4x4a, b4x4b);
  ternary_and(i3a, i3b);
  ternary_and(u4a, u4b);
  ternary_and(sa.a, sb.b);
#endif

#endif
}
