// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




SamplerState samp1;
Texture2DMS<float4> tex1;
Texture2DMSArray<float4> tex2;

float4 main(float4 a : A, out float4 b : SV_Target1) : SV_Target0
{
  float4 r = 0;
  r.xy += tex1.GetSamplePosition(a.z);
  r.xy += tex2.GetSamplePosition(a.w);
  r.xy += GetRenderTargetSamplePosition(3);
  r.xy += GetRenderTargetSamplePosition(a.y);
  b = r + 5;
  return r;
}
