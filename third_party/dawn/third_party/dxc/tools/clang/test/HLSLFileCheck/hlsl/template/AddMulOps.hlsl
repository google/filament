// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s -HV 2021 -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s /Zi | FileCheck %s

template<typename T>
T test_add(T t0, T t1) {
  return t0 + t1;
}

template<typename T>
T test_sub(T t0, T t1) {
  return t0 - t1;
}

template<typename T>
T test_mul(T t0, T t1) {
  return t0 * t1;
}

template<typename T>
T test_div(T t0, T t1) {
  return t0 / t1;
}

template<typename T>
T test_mod(T t0, T t1) {
  return t0 % t1;
}

struct S {
  int a;
  int4 a4;
};

int4 main(int4 a:A) : SV_Target {
  int i, j;
  unsigned int ui, uj;
  int1 i1 = 10, j1 = 11;
  int2 i2, j2;
  int3 i3, j3;
  int4 i4 = int4(1,2,3,4), j4 = int4(5,6,7,8);

  int1x1 i1x1, j1x1;
  int1x1 i1x2, j1x2;
  int1x1 i1x3, j1x3;
  int1x1 i1x4, j1x4;
  int1x1 i2x1, j2x1;

  float x,y;

  S s;

  bool b1, b2 = true;

  int arr1[7], arr2[7];

#ifdef CHECK_DIAGNOSTICS
  // DIAG-NOT: define void @main

  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  test_add(i, x);     // mismatched types
  test_add(s, j);     // mismatched types
  test_add(i, s);     // mismatched types
  test_add(i, uj);    // mismatched types
  test_add(b1,j);     // mismatched types
  test_add(s, s);     // can't test_add structs

  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  test_sub(i, x);     // mismatched types
  test_sub(i, x);     // mismatched types
  test_sub(s, j);     // mismatched types
  test_sub(i, s);     // mismatched types
  test_sub(i, uj);    // mismatched types
  test_sub(b1,j);     // mismatched types
  test_sub(s, s);     // can't test_sub structs

  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  test_mul(i, x);     // mismatched types
  test_mul(i, x);     // mismatched types
  test_mul(s, j);     // mismatched types
  test_mul(i, s);     // mismatched types
  test_mul(i, uj);    // mismatched types
  test_mul(b1,j);     // mismatched types
  test_mul(s, s);     // can't test_mul structs

  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  test_div(i, x);     // mismatched types
  test_div(i, x);     // mismatched types
  test_div(s, j);     // mismatched types
  test_div(i, s);     // mismatched types
  test_div(i, uj);    // mismatched types
  test_div(b1,j);     // mismatched types

  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  // DIAG: deduced conflicting types for parameter
  test_mod(i, x);     // mismatched types
  test_mod(i, x);     // mismatched types
  test_mod(s, j);     // mismatched types

  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  // DIAG: error: scalar, vector, or matrix expected
  test_mod(i, s);     // mismatched types
  test_mod(i, uj);    // mismatched types
  test_mod(b1,j);     // mismatched types
  test_mod(s, s);     // can't test_mod structs

  return 0;

#else
// These should all compile without diagnostics
// CHECK: define void @main

  int  r_i  = test_add(i,j) + test_sub(i,j) + test_mul(i,j) + test_div(i,j) + test_mod(i,j);
  int  r_ia  = test_add(1,5);
  int  r_ib  = test_add(i,int(6));
  int1 r_i1 = test_add(i1,j1) + test_sub(i1,j1) + test_mul(i1,j1) + test_div(i1,j1) + test_mod(i1,j1);
  int2 r_i2 = test_add(i2,j2) + test_sub(i2,j2) + test_mul(i2,j2) + test_div(i2,j2) + test_mod(i2,j2);
  int3 r_i3 = test_add(i3,j3) + test_sub(i3,j3) + test_mul(i3,j3) + test_div(i3,j3) + test_mod(i3,j3);
  int4 r_i4 = test_add(i4,j4) + test_sub(i4,j4) + test_mul(i4,j4) + test_div(i4,j4) + test_mod(i4,j4);
  int4 r_i4a = test_add(i4,int4(3,5,7,9));
  int4 r_i4b = test_add(int4(2,4,8,16), j4);
  s.a4 = test_add<int4>(int4(2,4,8,16), j4);
  s.a4 = test_add(int4(2,4,8,16), j4);

  int1x1 r_i1x1 = test_add(i1x1, j1x1) + test_sub(i1x1, j1x1) + test_mul(i1x1, j1x1) + test_div(i1x1, j1x1) + test_mod(i1x1, j1x1);
  int1x1 r_i1x2 = test_add(i1x2, j1x2);
  int1x1 r_i1x3 = test_add(i1x3, j1x3);
  int1x1 r_i1x4 = test_add(i1x4, j1x4);
  int1x1 r_i2x1 = test_add(i2x1, j2x1) + test_sub(i2x1, j2x1) + test_mul(i2x1, j2x1) + test_div(i2x1, j2x1) + test_mod(i2x1, j2x1);

  test_add(i, j);
  test_add(x, y);
  test_add(1, 2);
  test_add(i4.w, j);
  test_add(arr1[1], j);
  test_add(ui, uj);
  test_add(b1,b2);

  test_sub(i, j);
  test_sub(x, y);
  test_sub(1, 2);
  test_sub(i4.w, j);
  test_sub(arr1[1], j);
  test_sub(ui, uj);
  test_sub(b1,b2);

  test_mul(i, j);
  test_mul(x, y);
  test_mul(1, 2);
  test_mul(i4.w, j);
  test_mul(arr1[1], j);
  test_mul(ui, uj);
  test_mul(b1,b2);

  test_div(i, j);
  test_div(x, y);
  test_div(1, 2);
  test_div(i4.w, j);
  test_div(arr1[1], j);
  test_div(ui, uj);
  test_div(b1,b2);

  test_mod(i, j);
  test_mod(x, y);
  test_mod(1, 2);
  test_mod(i4.w, j);
  test_mod(arr1[1], j);
  test_mod(ui, uj);
  test_mod(b1,b2);

  return r_i4 + r_i4a + r_i4b;

#endif
}
