// RUN: %dxc -T cs_6_0 -E main -fcgl %s -spirv 2>&1 | FileCheck %s

// Check that the variable `value` is defined, and set to 6 in the entry point wrapper.
// CHECK: %value = OpVariable %_ptr_Private_uint Private
// CHECK: %main = OpFunction %void None
// CHECK-NEXT: OpLabel
// CHECK-NEXT: OpStore %value %uint_6

struct A {
    struct B { static const uint value = 6u; };
};

[[vk::binding(0,4)]] RWStructuredBuffer<uint> a;

[numthreads(1,1,1)]
void main() {
    a[0] = A::B::value;
}
