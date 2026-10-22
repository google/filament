// RUN: %dxc -T cs_6_0 -E main -spirv %s | FileCheck %s

struct X { int a1; };
struct Y : X { int2 a2; };
struct Z {
  X xs[2];
  Y y;
};

cbuffer CBStructs : register(b0) {
  Z z;
};
RWStructuredBuffer<int> Out : register(u1);

[numthreads(1,1,1)]
void main() {
// CHECK: OpAccessChain
// CHECK-SAME: %_ptr_Uniform_int
// CHECK-SAME: %CBStructs
// CHECK-SAME: %int_0
// CHECK-SAME: %int_1
// CHECK-SAME: %int_0
// CHECK-SAME: %int_0
 Out[0] = z.y.a1;
}
