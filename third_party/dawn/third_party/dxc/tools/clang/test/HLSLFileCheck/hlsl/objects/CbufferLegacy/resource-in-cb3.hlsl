// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: Tex1.0                              texture     f32          2d      T0             t0     1

SamplerState Samp;
cbuffer CB
{
  Texture2D Tex1[2];
  // Texture3D Tex2;
  // RWTexture2D<float4> RWTex1;
  // RWTexture3D<float4> RWTex2;
  // SamplerState Samp;
  float4 foo;
};

float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return Tex1[0].Sample(Samp, coord.xy) * foo;
}
