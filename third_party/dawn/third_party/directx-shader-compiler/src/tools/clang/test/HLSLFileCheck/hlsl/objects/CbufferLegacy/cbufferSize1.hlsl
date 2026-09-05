// RUN: %dxilver 1.6 | %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: error: CBuffer size is 65552 bytes, exceeding maximum of 65536 bytes.

cbuffer Foo1 : register(b5)
{
  float4 g1 : packoffset(c4096);
  float4 g2 : packoffset(c0);
}

float4 main() : SV_TARGET
{
  return g2+g1;
}
