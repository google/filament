// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a : A, int3 b : B) : SV_Target
{
  float s = 0;
  [loop]
  for(int i = 0; i < b.x; i++) {
    [flatten]
    if (b.z == 5)
      return s;

    [branch]
    if (b.z == 7)
      return s;

    s += a.x;
  }

  return s;
}
