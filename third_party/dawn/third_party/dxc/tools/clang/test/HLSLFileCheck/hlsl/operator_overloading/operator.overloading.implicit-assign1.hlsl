// RUN: %dxc -E main -T ps_6_0 -fcgl -HV 2021 %s | FileCheck %s

// CHECK: define internal void {{.*}}(%struct.MyArray* %this, %struct.MyArray* noalias sret %agg.result, %struct.MyArray* %RHS)

#define MAX_SIZE 100

struct MyArray {
  float4 A[MAX_SIZE];

  void splat(float f) {
    for (int i = 0; i < MAX_SIZE; i++)
      A[i] = f;
  };

  MyArray operator+(MyArray RHS) {
    MyArray OutArray;
    for (int i = 0; i < MAX_SIZE; i++)
      OutArray.A[i] = A[i] + RHS.A[i];
    return OutArray;
  };
};


float4 main(float4 col1: COLOR0, float4 col2: COLOR2, int ix : I) : SV_Target {
  MyArray A, B;
  B.splat(col1);
  A = A + B;
  return A.A[ix];
}
