// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

min16float g1[4], g2[8];

min16float main(min16float4 a : A, int b : B, int c : C) : SV_TARGET
{
  min16float x1[4];
  x1[0] = g1[b];
  x1[1] = g2[b];

  return x1[c];
}
