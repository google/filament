// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a : A, int3 b : B) : SV_Target
{
  float r;
  [branch]
  if (b.y)
    return a.y;

  [call]
  switch(b.x)
  {
  case 1:
    [branch]
    if (b.y)
      return a.y;
    r = 5.f;
    break;
  case 2:
    r = a.x;
    break;
  default:
    r = 3.f;
    break;
  }
  return r;
}
