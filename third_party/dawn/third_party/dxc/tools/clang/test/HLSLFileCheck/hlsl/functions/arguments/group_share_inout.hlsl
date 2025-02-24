// RUN: %dxc -E main -Tcs_6_0 -fcgl %s | FileCheck %s

// Make sure two alloca [5 x i32] in main for .
// CHECK:define void @main(
// CHECK:alloca [5 x i32]
// CHECK:alloca [5 x i32]
// CHECK:ret void
// CHECK:}

void foo(inout uint a[5], uint b) {
    a[0] = b;
    a[1] = b+1;
    a[2] = b+2;
    a[3] = b+3;
    a[4] = b+4;
}

uint bar(inout uint a[5], uint i) {
   return a[i];
}

groupshared uint ga[5];

RWBuffer<float> u;

[numthreads(8,8,1)]
void main(uint2 id : SV_DispatchThreadID) {
  foo(ga, id.x);
  u[id.x] = bar(ga, id.y);
}