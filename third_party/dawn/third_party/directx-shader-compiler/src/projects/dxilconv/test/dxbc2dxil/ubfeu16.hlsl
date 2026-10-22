// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted

min16uint x;

min16uint main() : SV_Target {
  min16uint p = 1 << 2;
  min16uint z = x / p * p;
  z += z ^ (z>>5);
  return z;
}
