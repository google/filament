// RUN: %dxc -T cs_6_0 -E main -fcgl %s -spirv 2>&1 | FileCheck %s

// Check that the folded constant value `6u` is stored into buffer.
// CHECK: %src_main = OpFunction %void None
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[BUFFER_A_0:%.*]] = OpAccessChain %_ptr_Uniform_uint %a %int_0 %uint_0
// CHECK-NEXT: OpStore [[BUFFER_A_0]] %uint_6

struct A {
    struct B { static const uint value = 6u; };
};

[[vk::binding(0,4)]] RWStructuredBuffer<uint> a;

[numthreads(1,1,1)]
void main() {
    a[0] = A::B::value;
}
