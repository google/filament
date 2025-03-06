// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float4 a : A, int3 b : B) : SV_Target
{
  float x = a.x * a.w + a.y * a.z;
  float y;

  [branch]
  if (b.x == 1)
  {
      y = x + 44.;
  }
  else
  {
      y = a.y - 77.;
  }

  return y * x;
}
