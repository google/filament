// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: var.res.Tex1

struct Resource
{
  Texture2D Tex1;
  // Texture3D Tex2;
  // RWTexture2D<float4> RWTex1;
  // RWTexture3D<float4> RWTex2;
  // SamplerState Samp;
  float4 foo;
};
struct MyStruct
{
  Resource res;
  int4 bar;
};

SamplerState Samp;
cbuffer CB
{
  MyStruct var;
};

float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return var.res.Tex1.Sample(Samp, coord.xy) * var.res.foo;
}
