// RUN: not %dxc -T vs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float4 f;
};

[[vk::push_constant]]
S pcs1;

[[vk::push_constant]] // error
S pcs2;

float main() : A {
    return 1.0;
}

// CHECK: :10:3: error: cannot have more than one push constant block
// CHECK: :7:3: note: push constant block previously defined here

