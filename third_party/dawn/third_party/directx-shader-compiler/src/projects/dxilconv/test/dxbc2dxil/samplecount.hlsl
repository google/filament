// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main() : SV_Target0
{
  float4 r = 0;
  r.xy += GetRenderTargetSampleCount();
  return r;
}
