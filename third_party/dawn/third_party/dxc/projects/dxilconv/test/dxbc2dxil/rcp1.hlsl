// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float3 main(float4 a : A) : SV_Target
{
  float3 r = 0;
  r += rcp(a.yyzx);
  return r;
}
