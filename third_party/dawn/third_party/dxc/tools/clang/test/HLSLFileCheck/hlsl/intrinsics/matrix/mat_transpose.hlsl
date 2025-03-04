// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure get cb0[0].y and cb0[1].y.
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
// CHECK: extractvalue %dx.types.CBufRet.f32 {{.*}}, 1
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
// CHECK: extractvalue %dx.types.CBufRet.f32 {{.*}}, 1

row_major float2x3 m;

float2 main(int i : A) : SV_TARGET
{
  return transpose(m)[1];
}
