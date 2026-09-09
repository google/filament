// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




uint4 main(uint4 a : AAA, uint4 b : BBB, uint4 c : CCC) : SV_Target
{
  double2 t1 = asdouble(a.xz, a.yw);
  double2 t2 = asdouble(b.xz, b.yw);
  double2 t3 = asdouble(c.xz, c.yw);
  float2 s1 = t1;
  int s2 = t2.xy;
  uint2 s3 = t3;
  double2 t4 = s1;
  double t5 = s2;
  double2 t6 = s3;
  double t7 = t4 + t5 + t5 + s1 + s2 + s3;
  uint4 ret;
  asuint(t7, ret.xy, ret.zw);
  return ret;
}
