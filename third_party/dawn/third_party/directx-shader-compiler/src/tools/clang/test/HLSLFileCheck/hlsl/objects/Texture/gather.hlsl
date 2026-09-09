// RUN: %dxc -E main -T ps_6_0 %s  | FileCheck %s

// CHECK: textureGather

SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 main(float2 a : A) : SV_Target
{
  uint status;
  float4 r = 0;
  r += text1.Gather(samp1, a);
  r += text1.Gather(samp1, a, uint2(-5, 7));
  r += text1.Gather(samp1, a, uint2(-3, 2), status); r += status;

  r += text1.GatherAlpha(samp1, a);
  r += text1.GatherAlpha(samp1, a, uint2(-3,7));
  r += text1.GatherAlpha(samp1, a, uint2(-3,7),status); r += status;
  r += text1.GatherAlpha(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));  
  r += text1.GatherAlpha(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=status;  
  
  r += text1.GatherBlue(samp1, a);
  r += text1.GatherBlue(samp1, a, uint2(-3,7));
  r += text1.GatherBlue(samp1, a, uint2(-3,7),status); r += status;
  r += text1.GatherBlue(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));  
  r += text1.GatherBlue(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=status;  
    
  r += text1.GatherGreen(samp1, a);
  r += text1.GatherGreen(samp1, a, uint2(-3,7));
  r += text1.GatherGreen(samp1, a, uint2(-3,7),status); r += status;
  r += text1.GatherGreen(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));  
  r += text1.GatherGreen(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=status;  
    
  r += text1.GatherRed(samp1, a);
  r += text1.GatherRed(samp1, a, uint2(-3,7));
  r += text1.GatherRed(samp1, a, uint2(-3,7),status); r += status;
  r += text1.GatherRed(samp1, a, uint2(-3,8),uint2(-2,3), uint2(-3,8),uint2(-2,3));  
  r += text1.GatherRed(samp1, a, uint2(-3,8),uint2(8,-3), uint2(8,-3), uint2(-3,2), status); r+=status;  
  
  return r;
}
