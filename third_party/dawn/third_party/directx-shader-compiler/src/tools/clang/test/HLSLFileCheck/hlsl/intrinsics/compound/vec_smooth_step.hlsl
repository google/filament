// RUN: %dxc -T ps_6_0 -E main %s | FileCheck %s

// Make sure use vector smoothstep works.
// CHECK: Saturate

float2 main(float2 a : A) : SV_TARGET
{
  return smoothstep(a, 1.2,1.6);
}