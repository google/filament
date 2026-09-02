// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float g1[6], g2[8];

float main(float4 a : A, int b : B, int c : C) : SV_TARGET
{
  float x1[6];
  x1[0] = g1[b];
  x1[1] = g2[b];
  x1[2] = g1[b+2];
  x1[3] = g1[b+3];
  x1[4] = g1[b+4];
  x1[5] = g1[b+5];

  return x1[c + b];
}
