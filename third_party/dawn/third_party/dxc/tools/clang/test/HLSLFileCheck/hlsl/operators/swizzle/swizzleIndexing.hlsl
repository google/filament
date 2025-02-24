// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

float4x4 m;
uint4 main(uint4 a : AAA, uint4 b : BBB) : SV_Target
{
  float4x4 tm = m;
  return a.wzyx[b.x] + tm._m00_m11[b.x];
}
