// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a : A, int3 b : B) : SV_Target
{
  float x;

  [branch]
  if (b.x == 1)
  {
    [branch]
    if (b.y)
      x = a.x + 5.;
    else
      x = a.x - 2.;
  }
  else
  {
    [branch]
    if (b.z)
      x = a.y - 77.;
  }

  return x;
}
