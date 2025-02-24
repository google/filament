// RUN: %dxc -T ps_6_1 -E main %s | FileCheck %s

// Make sure use lshr for uint vector.
// CHECK: lshr
// CHECK-NOT: ashr
// Make sure no and for src1 of lshr.
// CHECK-NOT: and
// CHECK-LABEL: ret


float main(uint2 a:A, uint b:B) : SV_Target {
  return (a>>b).y;
}