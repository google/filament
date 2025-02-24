// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1 : register(s5);
Texture2D<float4> text1 : register(t3);

float4 main(float2 a : A) : SV_Target
{
  uint status;
  float2 dx = a.yy, dy = a.yx;
  float4 r = 0;
  r += text1.SampleGrad(samp1, a, dx, dy);
  r += text1.SampleGrad(samp1, a, dx, dy, uint2(-5, 7));
  r += text1.SampleGrad(samp1, a, dx, dy, uint2(-4, 1), 0.5f);
  r += text1.SampleGrad(samp1, a, dx, dy, uint2(-3, 2), 0.f, status); r += status;
  r += text1.SampleGrad(samp1, a, dx, dy, uint2(-3, 2), a.x, status); r += status;
  return r;
}
