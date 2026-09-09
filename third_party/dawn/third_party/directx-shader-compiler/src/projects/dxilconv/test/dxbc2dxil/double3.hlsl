// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




bool b1;

bool main(uint2 a : AAA, uint2 b : BBB, uint2 c : CCC) : SV_Target
{
  double t1 = asdouble(a.x, a.y);
  double t2 = asdouble(b.x, b.y);
  double t3 = asdouble(c.x, c.y);
  double t4 = t1 + t2*t3;
  bool b1 = t1 == t2;
  bool b2 = t1 != t3;
  bool b3 = t2 < t3;
  bool b4 = t4 > t1;
  return b1 && b2 && b3 && b4;
}
