// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




uint4 main(uint4 a : AAA, uint4 b : BBB, uint4 c : CCC) : SV_Target
{
  double2 t1 = asdouble(a.xz, a.yw);
  double2 t2 = asdouble(b.xz, b.yw);
  double2 t3 = asdouble(c.xz, c.yw);
  double2 t4 = fma(t1, t2, t3);
  t4 = t4 + t1;
  t4 = saturate(t4);
  uint4 ret;
  asuint(t4, ret.xy, ret.zw);
  return ret;
}
