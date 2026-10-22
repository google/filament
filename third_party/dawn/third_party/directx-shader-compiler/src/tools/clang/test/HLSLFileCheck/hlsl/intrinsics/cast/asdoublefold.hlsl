// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// Verify that constants provided to asdouble can be folded
// CHECK-NOT: makeDouble
// CHECK: splitDouble.f64(i32 102, double -1.000000e+00


uint2 main() : SV_Target
{
  double d = asdouble(0, 0xBFF00000);
  uint2 ret;
  asuint(d, ret.x, ret.y);
  return ret;
}
