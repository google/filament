// RUN: %dxc -T lib_6_3  %s | FileCheck %s


// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H:%.*]], i32 2)
// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H]], i32 3)
// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H]], i32 4)

// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H]], i32 5)
// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H]], i32 6)
// CHECK: @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle [[H]], i32 7)


// CHECK: [[BCI:%.*]] = bitcast [24 x float]* {{.*}} to [2 x %class.matrix.float.4.3]*
// CHECK: call float @"\01?mat_array_test{{[@$?.A-Za-z0-9_]+}}"(<4 x float> {{.*}}, <4 x float> {{.*}}, [2 x %class.matrix.float.4.3]* [[BCI]]

float mat_array_test(in float4 inGBuffer0,
                                  in float4 inGBuffer1,
                                  float4x3 basisArray[2]);

cbuffer A {
float4 g0;
float4 g1;
float4x3 m[2];
};

[shader("pixel")]
float main() : SV_Target {
  return mat_array_test( g0, g1, m);
}
