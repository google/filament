// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Round_z

float test(float a)
{
  float b;
  modf(a, b);
  return b;
}


float4 main(float a : A) : SV_TARGET
{
   return test(a);
}