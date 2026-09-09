// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float3 main(float4 a : A) : SV_Target
{
  float3 r = 0;
  r += rcp(a.yyzx);
  return r;
}