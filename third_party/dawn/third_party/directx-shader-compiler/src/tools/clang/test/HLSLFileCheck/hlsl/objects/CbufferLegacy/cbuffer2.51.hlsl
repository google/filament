// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float4 g1;

float4 main() : SV_TARGET
{
  return g1.wyyy;
}
