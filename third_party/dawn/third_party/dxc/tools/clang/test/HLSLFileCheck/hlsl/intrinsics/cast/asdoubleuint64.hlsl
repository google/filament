// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Verify that i64 params to asdouble don't produce an invalid MakeDouble that takes i64
// CHECK: call double @dx.op.makeDouble.f64(i32 101, i32
// CHECK: SplitDouble

uint64_t i;

uint2 main() : SV_Target
{
  double d = asdouble(i&0xFFFFFFFF, i >> 32);
  uint2 ret;
  asuint(d, ret.x, ret.y);
  return ret;
}
