// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s 2>&1 | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG

// Check that HLSL bitwise assignment operators deal with dependent types

template<typename T, typename I>
void lshiftassign(inout T t, I i) {
  t <<= i;
}

template<typename T>
void lshiftassignby2(inout T t) {
  t <<= 2;
}

template<typename T, typename I>
T rshiftassign(T t, I i) {
  t >>= i;
  return t;
}

template<typename T0, typename T1>
T0 andassign(T0 t0, T1 t1) {
  t0 &= t1;
  return t0;
}

template<typename T0, typename T1>
T0 orassign(T0 t0, T1 t1) {
  t0 |= t1;
  return t0;
}

template<typename T0, typename T1>
T0 xorassign(T0 t0, T1 t1) {
  t0 ^= t1;
  return t0;
}

typedef struct {
  int4 a;
} S;

int4 main(int4 a:A) : SV_Target {
  int i = 7, j = 6;
  unsigned int ui = 7, uj = 6;
  int1 i1 = 10, j1 = 11;
  int4 i4 = int4(1,2,3,4), j4 = int4(5,6,7,8);
  int3x3 i3x3, j3x3;
  int iarr[7] = {1,2,3,4,5,6,7}, jarr[7] ;
  S s;
  bool b;
  bool1 b1;
  bool3 b3;
  bool2x2 b2x2;

  float x, y;
  float3 x3, y3;
  float4x4 x4x4, y4x4;

#ifdef CHECK_DIAGNOSTICS

  // DIG-NOT: define void @main


  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  lshiftassign(iarr,i);
  lshiftassign(i,iarr);
  lshiftassign(iarr,iarr);
  lshiftassignby2(iarr);

  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  lshiftassign(s,i);
  lshiftassign(i,s);
  lshiftassignby2(s);

  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  lshiftassign(x,ui);
  lshiftassign(i,x);
  lshiftassignby2(x);
  andassign(x,i);
  andassign(i,x);
  orassign(x,i);
  orassign(i,x);
  xorassign(x,i);
  xorassign(i,x);

  // DIAG: error: operator cannot be used with a bool lvalue
  // DIAG: error: operator cannot be used with a bool lvalue
  lshiftassign(b,i);
  rshiftassign(b,i);

  return 0;

#else

// CHECK: define void @main
// CHECK-NOT: error
// CHECK-NOT: warning

  lshiftassign(j,i);
  lshiftassign(uj,i);
  lshiftassign(j,ui);
  lshiftassign(i,b);

  lshiftassignby2(j);
  lshiftassignby2(ui);

  rshiftassign(j,i);
  rshiftassign(uj,i);
  rshiftassign(j,ui);
  rshiftassign(i,b);

  andassign(j,i);
  andassign(uj,i);
  andassign(j,ui);
  andassign(i,b);
  andassign(b,i);

  orassign(j,i);
  orassign(uj,i);
  orassign(j,ui);
  orassign(i,b);
  orassign(b,i);

  xorassign(j,i);
  xorassign(uj,i);
  xorassign(j,ui);
  xorassign(i,b);
  xorassign(b,i);

//  lshiftassign(i,3);
//  xorassign(b,2);

  return  0;

#endif
}
