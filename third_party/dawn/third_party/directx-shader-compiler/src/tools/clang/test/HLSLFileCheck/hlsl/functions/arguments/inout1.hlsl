// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: main

void test_out(out float4 m, float4 a) 
{
    m = abs(a*a.yxxx);
}

export void test_inout(inout float4 m, float4 a)
{
    m = abs(m+a*a.yxxx);
}

float4 main(float4 a : A, float4 b:B) : SV_TARGET
{
  float4 x = b;
  test_inout(x, a);
  test_out(x, b);
  return x;
}

