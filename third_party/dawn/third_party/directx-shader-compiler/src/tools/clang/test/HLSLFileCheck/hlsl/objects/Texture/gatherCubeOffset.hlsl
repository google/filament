// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: textureGather
// CHECK: i32 0, i32 0, i32 0)

// CHECK: textureGather
// CHECK: i32 -5, i32 7, i32 1)

// CHECK: textureGather
// CHECK: i32 -3, i32 2, i32 3)

// CHECK: textureGather
// CHECK: i32 -3, i32 2, i32 2)

// CHECK: textureGather
// CHECK: i32 undef, i32 undef, i32 2)


SamplerState samp1;
Texture2D<float4> text1;
Texture2DArray<float4> text2;
TextureCubeArray<float4> text3;

float4 main(float4 a : A) : SV_Target
{
  uint status;
  float4 r = 0;
  r += text1.GatherRed(samp1, a.xy);
  r += text1.GatherGreen(samp1, a.xy, uint2(-5, 7));
  r += text1.GatherAlpha(samp1, a.xy, uint2(-3, 2), status); r += status;

  r += text2.GatherBlue(samp1, a.xyz, uint2(-3, 2), status); r += status;
  r += text3.GatherBlue(samp1, a, status); r += status;
  return r;
}
