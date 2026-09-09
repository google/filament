// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
Texture2D<float4> srv1 : register(t3);

float4 main(float3 a : A, float2 b : B) : SV_Target
{
  uint status;
  uint2 offset = uint2(-5, 7);
  float4 r = 0;
  r += srv1.Load(a);
  r += srv1[b];
  r += srv1.Load(a, offset);
  r += srv1.Load(a, offset, status); r += status;
  r += srv1.Load(a, uint2(0,0), status); r += status;
  return r;
}
