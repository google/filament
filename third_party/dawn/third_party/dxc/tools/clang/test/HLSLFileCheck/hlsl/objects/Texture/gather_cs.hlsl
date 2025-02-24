// RUN: %dxc -E main -T cs_6_0 %s  | FileCheck %s

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

struct InOut {
  float2 coord;
  float4 result;
};

RWStructuredBuffer<InOut> Data;

// CHECK: define void @main()

[numthreads(64,1,1)]
void main(uint id : SV_GroupIndex)
{
  float2 a = Data[id].coord;
  uint status;
  float4 r = 0;

  r += text1.Gather(samp1, a);
  r += text1.Gather(samp1, a, uint2(-5, 7));
  r += text1.Gather(samp1, a, uint2(-3, 2), status); r += CheckAccessFullyMapped(status);

  a *= 1.125; // Prevent GatherCmpRed from being optimized to equivalent GatherCmp above

  r += text1.GatherAlpha(samp1, a);
  r += text1.GatherAlpha(samp1, a, uint2(-3,7));
  r += text1.GatherAlpha(samp1, a, uint2(-3,7),status); r += CheckAccessFullyMapped(status);
  r += text1.GatherAlpha(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));
  r += text1.GatherAlpha(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=CheckAccessFullyMapped(status);

  r += text1.GatherBlue(samp1, a);
  r += text1.GatherBlue(samp1, a, uint2(-3,7));
  r += text1.GatherBlue(samp1, a, uint2(-3,7),status); r += CheckAccessFullyMapped(status);
  r += text1.GatherBlue(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));
  r += text1.GatherBlue(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=CheckAccessFullyMapped(status);

  r += text1.GatherGreen(samp1, a);
  r += text1.GatherGreen(samp1, a, uint2(-3,7));
  r += text1.GatherGreen(samp1, a, uint2(-3,7),status); r += CheckAccessFullyMapped(status);
  r += text1.GatherGreen(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));
  r += text1.GatherGreen(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=CheckAccessFullyMapped(status);

  r += text1.GatherRed(samp1, a);
  r += text1.GatherRed(samp1, a, uint2(-3,7));
  r += text1.GatherRed(samp1, a, uint2(-3,7),status); r += CheckAccessFullyMapped(status);
  r += text1.GatherRed(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));
  r += text1.GatherRed(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=CheckAccessFullyMapped(status);

  Data[id].result = r;
}
