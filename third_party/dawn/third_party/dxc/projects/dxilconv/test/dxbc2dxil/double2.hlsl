// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




bool b1;

uint2 main(uint2 a : AAA, uint2 b : BBB) : SV_Target
{
  double t1 = asdouble(a.x, a.y);
  double t2 = asdouble(b.x, b.y);
  double t3 = t1 + t2;
  t3 += 0.5;
  t3 *= t1;
  t3 = t2 / t1;
  t3 = min(t3, 1.77);
  t3 = max(t3, t2-4.35);
  t3 = b1 ? abs(t3) : t3;
  uint2 ret;
  asuint(t3, ret.x, ret.y);
  return ret;
}
