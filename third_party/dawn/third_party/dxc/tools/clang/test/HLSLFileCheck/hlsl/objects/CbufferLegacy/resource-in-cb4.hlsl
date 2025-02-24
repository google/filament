// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK:Index for resource array inside cbuffer must be a literal expression


SamplerState Samp;

cbuffer CB
{
  Texture2D<float4> Tex1[2][2];
  // Texture3D Tex2;
  // RWTexture2D<float4> RWTex1;
  // RWTexture3D<float4> RWTex2;
  // SamplerState Samp;
  float4 foo;
};

uint i;

float4 main(int4 a : A, float2 coord : TEXCOORD) : SV_TARGET
{
  return Tex1[0][i].Sample(Samp, coord) * foo;
}
