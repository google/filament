// RUN: %dxc -E main -Tps_6_0 -fcgl %s | FileCheck %s


// Make sure only one alloca [5 x i32], and none for nested call.
// CHECK:define float @main(
// CHECK:alloca [5 x i32]
// CHECK-NOT:alloca [5 x i32]

void foo(inout uint a[5], uint b) {
    a[0] = b;
    a[1] = b+1;
    a[2] = b+2;
    a[3] = b+3;
    a[4] = b+4;
}

uint bar(inout uint a[5], uint2 i) {
  foo(a, i.x);
  return a[i.y];
}

float main(uint2 i:A) : SV_Target {
  uint a[5];
  return bar(a, i);
}