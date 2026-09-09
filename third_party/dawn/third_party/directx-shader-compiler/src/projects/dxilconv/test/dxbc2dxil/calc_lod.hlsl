// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1;

Texture2D<float4> tex1;
Texture2DArray<float4> tex2;
TextureCubeArray<float4> tex3;

float4 main(float4 a : A) : SV_Target
{
  float4 r = 0;
  r += tex1.CalculateLevelOfDetail(samp1, a.xy);  // sampler, coordinates
  r += tex2.CalculateLevelOfDetail(samp1, a.xy);
  r += tex3.CalculateLevelOfDetail(samp1, a.xyz);

  return r;
}
