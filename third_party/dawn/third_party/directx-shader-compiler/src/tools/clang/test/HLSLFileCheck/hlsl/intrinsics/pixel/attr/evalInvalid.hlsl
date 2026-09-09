// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: attribute evaluation can only be done on values taken directly from inputs

float4 color;
float4 main(float4 a : A) : SV_Target
{
  float4 r = EvaluateAttributeCentroid(color);

  return r;
}