// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : A) : SV_Target
{
  float4 r = 0;

  if (a.x > 0.3)
    discard;

  if (a.y != 0)
    discard;

  return r;
}
