// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 main(float2 a : A) : SV_Target
{
  uint status;
  float bias = a.y;
  float4 r = 0;
  r += text1.SampleBias(samp1, a, bias);
  r += text1.SampleBias(samp1, a, bias, uint2(-5, 7));
  r += text1.SampleBias(samp1, a, bias, uint2(-4, 1), 0.5f);
  r += text1.SampleBias(samp1, a, bias, uint2(-3, 2), 0.f, status); r += status;
  r += text1.SampleBias(samp1, a, bias, uint2(-3, 2), a.x, status); r += status;
  return r;
}
