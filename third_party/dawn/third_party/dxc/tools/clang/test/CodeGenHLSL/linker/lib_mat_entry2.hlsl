// RUN: %dxc -T lib_6_3  %s | FileCheck %s

// CHECK: [[BCI:%.*]] = bitcast <12 x float>* {{.*}} to %class.matrix.float.4.3*
// CHECK:call <3 x float> @"\01?mat_test{{[@$?.A-Za-z0-9_]+}}"(<4 x float> {{.*}}, <4 x float> {{.*}}, %class.matrix.float.4.3* {{.*}}[[BCI]])

float3 mat_test(in float4 in0,
                                  in float4 in1,
                                  inout float4x3 m);

cbuffer A {
float4 g0;
float4 g1;
float4x3 M;
};

[shader("pixel")]
float3 main() : SV_Target {
  float4x3 m = M;
  return mat_test( g0, g1, m);
}
