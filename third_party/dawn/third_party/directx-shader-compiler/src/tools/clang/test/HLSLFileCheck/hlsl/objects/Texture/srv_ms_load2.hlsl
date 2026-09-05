// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: textureLoad

Texture2DMS<float3,8> srv1 : register(t3);
Texture2DMS<float3,8> srv2[8] : register(t4);

float3 main(int2 a : A, int c : C, int2 b : B) : SV_Target
{
  uint status;
  uint2 offset = uint2(-5, 7);
  float3 r = 0;
  r += srv1.sample[2][b];
  r += srv2[6].sample[2][offset];
  return r;
}
