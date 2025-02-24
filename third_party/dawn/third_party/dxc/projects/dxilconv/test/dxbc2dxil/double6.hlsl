// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




uint g_Idx1, g_Idx2;

double g2[4];
static double g3[6] = {8., 1., 2., 3.56, 7., 33.};

uint4 main(uint4 a : AAA, uint4 b : BBB, uint4 c : CCC) : SV_Target
{
  double g1[4] = {1., 2., 3.56, 7.};
  double2 t1 = asdouble(a.xz, a.yw);
  double2 t2 = asdouble(b.xz, b.yw);
  double2 t3 = asdouble(c.xz, c.yw);
  double t7 = t1 + t2 + t3;
  t7 += g1[g_Idx1] + g2[g_Idx1] + g3[g_Idx2];
  g1[g_Idx2] = t7;
  t7 += g1[g_Idx1];
  uint4 ret;
  asuint(t7, ret.xy, ret.zw);
  return ret;
}
