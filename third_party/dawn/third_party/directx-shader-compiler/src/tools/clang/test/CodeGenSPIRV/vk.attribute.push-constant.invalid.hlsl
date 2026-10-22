// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float f;
};

[[vk::push_constant, vk::binding(5)]]
S pcs;

float main() : A {
    return 1.0;
}

// CHECK: :7:3: error: vk::push_constant attribute cannot be used together with vk::binding attribute
