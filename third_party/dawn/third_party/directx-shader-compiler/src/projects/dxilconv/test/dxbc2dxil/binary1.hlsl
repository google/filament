// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float main(float a : A, float b : B, float2 c : C) : SV_Target
{
  float r = a;
  r += a;
  r /= a;
  r *= b;
  r = max(r, c.x);
  r = min(r, c.y);
  return r;
}
