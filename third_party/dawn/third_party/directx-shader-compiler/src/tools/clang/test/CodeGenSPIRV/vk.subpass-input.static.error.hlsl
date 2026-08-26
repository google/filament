// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float  a;
    float2 b;
    float  c;
};

[[vk::input_attachment_index(1)]]
static SubpassInput SI2; // error

void main() {

}

// CHECK: :10:21: error: SubpassInput(MS) must be externally visible
