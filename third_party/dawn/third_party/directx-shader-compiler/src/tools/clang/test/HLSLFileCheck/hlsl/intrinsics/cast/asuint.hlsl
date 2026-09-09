// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: MakeDouble
// CHECK: SplitDouble

uint2 main(uint2 a : A, uint2 b : B, uint2 c : C) : SV_TARGET
{
  uint x, y;
  asuint(fma(asdouble(a.x, a.y), asdouble(b.x, b.y), asdouble(c.x, c.y)), x, y);
  return uint2(x,y);
}
