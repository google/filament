// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s


// Make sure there're select for atan2.
// CHECK: Atan
// CHECK: select
// CHECK: select
// CHECK: select
// CHECK: select

float main(float2 a : A) : SV_Target {
  return atan2(a.y, a.x);
}