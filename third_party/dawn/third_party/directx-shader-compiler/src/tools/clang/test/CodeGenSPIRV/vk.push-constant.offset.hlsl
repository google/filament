// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpMemberDecorate %type_PushConstant_S 0 Offset 0
// CHECK: OpMemberDecorate %type_PushConstant_S 1 Offset 8
// CHECK: OpMemberDecorate %type_PushConstant_S 2 Offset 32

struct S {
    float a;
    [[vk::offset(8)]]
    float2 b;
    [[vk::offset(32)]]
    float4 f;
};

[[vk::push_constant]]
S pcs1;

float main() : A {
    return 1.0;
}
