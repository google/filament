// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float4x4 a : A, int4 b : B) : SV_Target
{
  return a[b.x][b.y];
}
