// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




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
