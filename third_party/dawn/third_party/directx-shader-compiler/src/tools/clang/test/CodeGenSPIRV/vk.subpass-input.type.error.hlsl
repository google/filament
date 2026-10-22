// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct S {
    float  a;
    float2 b;
    float  c;
};

[[vk::input_attachment_index(0)]]
SubpassInput<S> SI1; // error

void main() {

}

// CHECK: :10:17: error: only scalar/vector types allowed as SubpassInput(MS) parameter type
