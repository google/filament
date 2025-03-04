// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: evalCentroid

float4 main(float4x4 a : A) : SV_Target
{
  float4 r = EvaluateAttributeCentroid(a)[0];

  return r;
}
