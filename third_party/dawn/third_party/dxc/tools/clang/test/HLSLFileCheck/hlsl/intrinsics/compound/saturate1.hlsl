// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float3 main(float3 a : A) : SV_Target
{
  return saturate(a.xzx);
}
