// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG


// pass - bool, int, float, vector, matrix
// fail - struct, array

template<typename T>
bool my_any(T t0) {
  return any(t0);
}

template<typename T>
bool my_all(T t0) {
  return all(t0);
}

template<typename T>
bool any_lessthan(T t0, T t1) {
  return any(t0 < t1);
}

template<typename T>
bool all_lessthan(T t0, T t1) {
  return all(t0 < t1);
}

struct S {
   bool4 b;
   int i;
};

bool main(int4 a:A) : SV_Target {
  int i = 7, j = 6;
  int1 i1 = 10, j1 = 11;
  int4 i4 = int4(1,2,3,4), j4 = int4(5,6,7,8);
  int3x3 i3x3, j3x3;
  int iarr[7] = {1,2,3,4,5,6,7}, jarr[7] ;
  bool barr[6];
  S s1, s2;

#ifdef CHECK_DIAGNOSTICS

  // DIAG-NOT: define void @main

  // DIAG: not viable: no known conversion from 'S' to 'float'
  // DIAG: not viable: no known conversion from 'int [7]' to 'int'
  // DIAG: not viable: no known conversion from 'bool [6]' to 'bool'
  my_any(s1);
  my_any(iarr);
  my_any(barr);

  // DIAG: not viable: no known conversion from 'S' to 'float'
  // DIAG: not viable: no known conversion from 'int [7]' to 'int'
  // DIAG: not viable: no known conversion from 'bool [6]' to 'bool'
  my_all(s1);
  my_all(iarr);
  my_all(barr);

  return true;

#else

  // CHECK: define void @main

  my_any(i);
  my_any(i1);
  my_any(i4);
  my_any(i3x3);

  bool b1 = all_lessthan(i,j);
  bool b2 = any_lessthan(i,j);
  bool b3 = any_lessthan(i1,j1);
  bool b4 = all_lessthan(i1,j1);
  bool b5 = any_lessthan(i4,j4);
  bool b6 = all_lessthan(i4,j4);
  bool b7 = any_lessthan(i3x3,j3x3);
  bool b8 = all_lessthan(i3x3,j3x3);
  bool b9 = all_lessthan(i,i4.x);
  bool b9a = any_lessthan(i,j4[2]);

  float x, y;
  float3 x3, y3;
  float4x4 x4x4, y4x4;

  bool b10 = any_lessthan(x,y);
  bool b11 = all_lessthan(x,y);
  bool b12 = any_lessthan(x3,y3);
  bool b13 = all_lessthan(x3,y3);
  bool b14 = any_lessthan(x4x4,y4x4);
  bool b15 = all_lessthan(x4x4,y4x4);
  bool b16 = any_lessthan(x3,float3(0.1, 2.7, 3.3));

  bool b20 = all_lessthan(s1.i,s2.i);
  bool b21 = all_lessthan(x4x4[1],y4x4[3].zwyx);

  return b1 || b2 || b3 || b4 || b5 || b6 || b7 || b8  || b9 || b10 || b11 || b12 || b13 || b14 || b15 || b16;

#endif

}





