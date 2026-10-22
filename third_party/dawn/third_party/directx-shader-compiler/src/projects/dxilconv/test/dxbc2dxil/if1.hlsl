// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a : A, int b : B) : SV_Target
{
  [branch]
  if (b == 1)
    return a.x;
  else
    return a.y;
}
