// RUN: %dxc -E main -T ps_6_0 -HV 2018 %s | FileCheck %s

// CHECK: fcmp
// CHECK: fcmp
// CHECK: fcmp
// CHECK: fcmp
// CHECK: select
// CHECK: select
// CHECK: select
// CHECK: select

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
  return (a >= b) ? 1.0f : 0.0f;
}
