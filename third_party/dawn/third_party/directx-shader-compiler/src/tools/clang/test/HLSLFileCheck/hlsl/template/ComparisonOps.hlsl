// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG


template<typename T0, typename T1>
T0 lessthan(T1 t0, T1 t1) {
  return t0 < t1;
}

typedef struct {
  int4 a, b;
} S;


int4 main() : SV_Target {
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

// DIAG: error: scalar, vector, or matrix expected
// DIAG-NOT: define void @main

  return lessthan<bool>(sa, sb);

#else

// CHECK: define void @main

  lessthan<bool>(ia, ib);

  return lessthan<bool4>(int4(1,3,7,11),int4(10,8,6,5));

#endif
}
