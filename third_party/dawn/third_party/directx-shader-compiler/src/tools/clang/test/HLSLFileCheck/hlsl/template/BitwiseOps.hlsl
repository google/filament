// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s 2>&1 | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG

// Check that HLSL bitwise operators deal with dependent types

template<typename T>
T Not(T t) {
  return ~t;
}

template<typename T>
T And(T t0, T t1) {
  return t0 & t1;
}

template<typename T>
T Or(T t0, T t1) {
  return t0 | t1;
}

template<typename T, typename I>
T lshift(T t, I i) {
  T r = t << i;
  return r;
}

template<typename T, typename I>
T rshift(T t, I i) {
  return t >> i;
}

typedef struct {
  int4 a;
} S;

int4 main(int4 a:A) : SV_Target {
  int i = -7, j = 6;
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

  // DIAG-NOT: define void @main

  // DIAG: function cannot return array type
  // DIAG: function cannot return array type
  // DIAG: function cannot return array type
  // DIAG: function cannot return array type
  // DIAG: function cannot return array type
  Not(iarr);
  And(iarr,iarr);
  Or(iarr,iarr);
  lshift(iarr,i);
  rshift(iarr,i);

  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  Not(s);
  And(s,s);
  Or(s,s);
  lshift(s,i);
  rshift(s,i);

  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  // DIAG: error: int or unsigned int type required
  Not(x);
  And(x,x);
  Or(x,x);
  lshift(x,i);
  lshift(i,x);
  rshift(x,i);
  rshift(ui,x);

  // DIAG-NOT: warning
  // DIAG-NOT: error

  return 0;

#else

// CHECK: define void @main
  int r1 = Not(i);
  int r2 = Not(i1);
  int4 r3 = Not(i4);
  int3x3 r4 = Not(i3x3);
  int r5 = Not(iarr[5]);
  int r6 = Not(i4.w);
  int4 r7 = Not(s.a);
  bool3 b3b = Not(b3);
  Not(ui);
  Not(b);
  And(b,b);
  Or(b,b);

  // Note that << and >> allow bool LHS operands, but <<= and >>= give "error: operator cannot be used with a bool lvalue"
  lshift(b,i);
  lshift(i,b);
  rshift(b,i);
  rshift(i,b);

  return  r1 + r2 + r3.x + r4[3].y + r5 + r6;

#endif

}
