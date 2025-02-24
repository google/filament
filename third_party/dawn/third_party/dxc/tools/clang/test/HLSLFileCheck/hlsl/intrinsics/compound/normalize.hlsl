// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Dot4
// CHECK: Rsqrt

float4 main(float4 a: A) : SV_TARGET
{
  return normalize(a);
}
