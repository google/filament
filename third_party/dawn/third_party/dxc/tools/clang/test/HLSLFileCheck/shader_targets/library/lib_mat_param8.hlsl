// RUN: %dxc -T lib_6_3 -default-linkage external %s | FileCheck %s

// Make sure return matrix struct works.
// CHECK-DAG: bitcast %class.matrix.float.3.2* {{.*}} to <6 x float>*
// CHECK-DAG: bitcast %class.matrix.float.2.3* {{.*}} to <6 x float>*
// CHECK-DAG: bitcast %class.matrix.float.3.2* {{.*}} to <6 x float>*
// CHECK-DAG: bitcast %class.matrix.float.2.3* {{.*}} to <6 x float>*

struct MA {
  float2x3 ma[2];
};

MA mat_test2( float3x2 m[2], int idx) {
  MA ma = { { transpose(m[0]), transpose(m[1])}};
  return ma;
}