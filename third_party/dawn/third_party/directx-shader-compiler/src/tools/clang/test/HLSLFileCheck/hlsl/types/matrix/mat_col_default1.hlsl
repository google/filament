// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Make sure get cb0[1].y and cb0[1].z.
// CHECK: call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32
// CHECK: extractvalue %dx.types.CBufRet.f32 {{.*}}, 1
// CHECK: extractvalue %dx.types.CBufRet.f32 {{.*}}, 2

cbuffer Transform : register(b0)
{
  float4 transformRows[3];
}

float2 main(int i : A) : SV_TARGET
{
  float3x4 mat;
  mat[0] = transformRows[0];
  mat[1] = transformRows[1];
  mat[2] = transformRows[2];
  return mat[1].yz;
}
