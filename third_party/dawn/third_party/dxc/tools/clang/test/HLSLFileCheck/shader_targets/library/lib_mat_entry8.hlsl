// RUN: %dxc -T lib_6_3  %s | FileCheck %s

// Make sure sure major change on matrix array works.
// CHECK: bitcast [12 x float]* {{.*}} to [2 x %class.matrix.float.3.2]*
// CHECK: bitcast [12 x float]* {{.*}} to [2 x %class.matrix.float.3.2]*
// CHECK: bitcast %class.matrix.float.2.3* {{.*}} to <6 x float>*
// CHECK: bitcast %class.matrix.float.2.3* {{.*}} to <6 x float>*


struct MA {
  float2x3 ma[2];
};

MA mat_test2( float3x2 m[2], int idx);

cbuffer A {
column_major float3x2 cma[2];
row_major    float3x2 rma[2];
uint3 i;
};


[shader("pixel")]
float3 mainx() : SV_Target {
  MA ma = mat_test2(cma, i.x);
  MA ma2 = mat_test2(rma, i.y);
  return ma.ma[i.z][i.y] + ma2.ma[i.z][i.y];
}