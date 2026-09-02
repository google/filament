// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1 : register(s5);
Texture2D<snorm float4> text1 : register(t3);
Texture2D<unorm float3> text2 : register(t7);

float4 main(float2 a : A) : SV_Target
{
  return text1.Sample(samp1, a) + text2.Sample(samp1, a).zzyx;
}
