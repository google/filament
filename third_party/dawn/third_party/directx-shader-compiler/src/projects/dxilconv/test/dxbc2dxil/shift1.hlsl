// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

int main(int a : A, uint b : B, int c : C) : SV_TARGET
{
  return (a << 77) + (a >> 3) + (b >> 8) + (a << c) + (a >> c) + (b >> c);
}
