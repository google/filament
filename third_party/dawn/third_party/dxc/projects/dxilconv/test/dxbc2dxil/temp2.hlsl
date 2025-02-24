// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




int4 main(int4 a : A, int4 b : B) : SV_TARGET
{
  int4 c = a + b;
  return c + int4(0,1,2,3);
}
