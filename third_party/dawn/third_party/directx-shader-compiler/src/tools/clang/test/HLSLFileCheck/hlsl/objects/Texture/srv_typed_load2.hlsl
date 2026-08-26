// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
Buffer<float4> buf1 : register(t3);

float4 main(int a : A) : SV_Target
{
  uint status;
  float4 r = 0;
  r += buf1.Load(a);
  r += buf1[a];
  r += buf1.Load(a, status); r += status;
  return r;
}
