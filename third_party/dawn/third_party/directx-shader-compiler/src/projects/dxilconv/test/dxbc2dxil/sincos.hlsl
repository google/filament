// FXC command line: fxc /T vs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : AAA) : SV_Position
{
  float4 s, s2, c, c2;
  sincos(a, s, c);
  sincos(a*2.f, s2, c2);
  return s + c + s2.z;
}
