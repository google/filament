// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float2 a : A, int3 b : B) : SV_Target
{
  float r;
  switch(b.x)
  {
  case 1:
    r = 5.f;
    if(b.y == 11)
      break;
    r = 5.5f;
    break;
  case 2:
    [flatten]
    if(b.y == 12)
      break;
    r = a.x;
    break;
  default:
    r = 3.f;
    break;
  }
  return r;
}
