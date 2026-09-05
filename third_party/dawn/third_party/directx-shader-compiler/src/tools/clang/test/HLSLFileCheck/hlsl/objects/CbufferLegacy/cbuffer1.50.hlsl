// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
cbuffer Foo1 : register(b5)
{
  float4 g1;
}
cbuffer Foo2 : register(b5)
{
  float4 g2;
}

float4 main() : SV_TARGET
{
  return g2;
}
