// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);
TextureCubeArray<float4> text2 : register(t5);

float4 main(float4 a : A) : SV_Target
{
  uint status;
  float level = a.y;
  float4 r = 0;
  r += text1.SampleLevel(samp1, a.xy, level);
  r += text1.SampleLevel(samp1, a.xy, level, uint2(-5, 7));
  r += text1.SampleLevel(samp1, a.xy, level, uint2(-4, 1));
  r += text1.SampleLevel(samp1, a.xy, level, uint2(-3, 2), status); r += status;
  r += text1.SampleLevel(samp1, a.xy, level, uint2(-3, 2), status); r += status;

  r += text2.SampleLevel(samp1, a, level);
  r += text2.SampleLevel(samp1, a, level);
  r += text2.SampleLevel(samp1, a, level);
  r += text2.SampleLevel(samp1, a, level, status); r += status;
  r += text2.SampleLevel(samp1, a, level, status); r += status;
  return r;
}
