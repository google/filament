// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

float4 main(int4 a : A) : SV_TARGET
{
  return a + abs(a.yxxx);
}