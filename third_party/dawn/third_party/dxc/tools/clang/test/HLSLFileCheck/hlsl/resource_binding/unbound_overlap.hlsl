// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck -input-file=stderr %s


// CHECK: error: more than one unbounded resource (Tex1 and Tex2) in space 0
// CHECK: error: more than one unbounded resource (Tex3 and Tex4) in space 1


Texture2D<float4> Tex1[] : register(t3);  // unbounded, overlap in space0
Texture2D<float4> Tex2[] : register(t4);  // unbounded, overlap in space0
Texture2D<float4> Tex3[] : register(t5, space1);  // unbounded, overlap in space1
Texture2D<float4> Tex4[] : register(t6, space1);  // unbounded, overlap in space1
Texture2D<float4> Tex5[] : register(t7, space2);  // unbounded, no overlap
Texture2D<float4> Tex6[] : register(t8, space3);  // unbounded, no overlap

SamplerState Samp : register(s0);


float4 main(int4 a : A, float4 coord : TEXCOORD) : SV_TARGET
{
  return (float4)1.0
  * Tex1[0].Sample(Samp, coord.xy)
  * Tex2[0].Sample(Samp, coord.xy)
  * Tex3[0].Sample(Samp, coord.xy)
  * Tex4[0].Sample(Samp, coord.xy)
  * Tex5[0].Sample(Samp, coord.xy)
  * Tex6[0].Sample(Samp, coord.xy)
  ;
}
