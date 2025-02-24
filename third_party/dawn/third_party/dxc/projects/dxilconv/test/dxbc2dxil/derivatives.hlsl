// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(float4 a : A, float4 b : B) : SV_Target
{
  float4 r = 0;

  r += ddx(b);
  r += ddy(b);

  r += ddx_coarse(a);
  r += ddy_coarse(a);

  r += ddx_fine(a);
  r += ddy_fine(a);

  return r;
}
