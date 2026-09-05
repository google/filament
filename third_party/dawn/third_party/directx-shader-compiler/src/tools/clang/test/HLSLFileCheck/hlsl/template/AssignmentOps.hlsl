// RUN: %dxc -E main -T ps_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s -HV 2021 -DCHECK_DIAGNOSTICS | FileCheck %s -check-prefix=DIAG

template<typename T0, typename T1>
void assign(inout T0 t0, T1 t1) {
  t0 = t1;
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
  S s, t;
  float4x3 f4arr[11];

#ifdef CHECK_DIAGNOSTICS

  // DIAG-NOT: define void @main
  // DIAG: cannot implicitly convert
  assign(f4arr, x);
  // DIAG: warning: implicit truncation of vector type
  // DIAG: warning: implicit truncation of vector type
  assign(i, i4);
  assign(b, b2);
  // DIAG-NOT: error

#else

// CHECK: define void @main

  assign(i, j);
  assign(i, i1);
  assign(i, i1);
  assign(i, i1);

  assign(i2, i);
  assign(s, t);
  assign(s.i4, i4);

#endif

}
