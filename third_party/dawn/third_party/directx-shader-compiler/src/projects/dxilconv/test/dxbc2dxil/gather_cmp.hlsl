// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerComparisonState samp1;
Texture2D<float4> text1;
Texture2DArray<float4> text2;
TextureCubeArray<float4> text3;

float4 main(float4 a : A) : SV_Target
{
  uint status;
  float cmp = a.y;
  float4 r = 0;
  r += text1.GatherCmpRed(samp1, a.xy, cmp);
  r += text1.GatherCmpGreen(samp1, a.xy, cmp, uint2(-5, 7));
  r += text1.GatherCmpAlpha(samp1, a.xy, cmp, uint2(-3, 2), status); r += status;

  r += text2.GatherCmpBlue(samp1, a.xyz, cmp, uint2(-3, 2), status); r += status;
  r += text3.GatherCmpBlue(samp1, a, cmp, status); r += status;
  return r;
}
