// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: ps_6_0 does not support pull-model evaluation of position

float4 main(float4 a : SV_POSITION) : SV_Target
{
  float4 r = EvaluateAttributeCentroid(a.yxzw);

  return r;
}