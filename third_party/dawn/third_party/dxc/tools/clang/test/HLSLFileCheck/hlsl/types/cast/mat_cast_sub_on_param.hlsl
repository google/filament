// RUN: %dxc /T ps_6_0 /E main %s | FileCheck %s

// Make sure cast then subscript on parameter works.

float4x4 a;
float4x4 b;

float3 mat_cast(float4x4 m) {
  return ((float3x3)m)[0];
}


float3 main() :SV_Target {
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 0)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 1)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 2)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 4)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 5)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0
  // CHECK:call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, {{.*}}, i32 6)
  // CHECK:extractvalue %dx.types.CBufRet.f32 {{.*}}, 0

  return mat_cast(a+b);
}