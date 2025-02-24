// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
cbuffer Foo
{
  float4 g1[16];
};

cbuffer Bar
{
  uint3 idx[8];
};

float4 main(int2 a : A) : SV_TARGET
{
  return g1[idx[a.x].z].wyyy;
}
