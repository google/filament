// RUN: %dxc -E main -T ps_6_0 %s

cbuffer Foo1 : register(b5)
{
  float4 g1 : packoffset(c1);
  float4 g2 : packoffset(c0);
}

float4 main() : SV_TARGET
{
  return g2+g1;
}
