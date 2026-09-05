// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: @main

SamplerState Samp;
extern  Texture2D Tex1;

  float4 foo;


float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return Tex1.Sample(Samp, coord.xy) * foo;
}
