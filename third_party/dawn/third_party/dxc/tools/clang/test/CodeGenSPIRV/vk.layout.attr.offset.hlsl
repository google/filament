// RUN: %dxc -T vs_6_0 -E main -fvk-use-dx-layout -fcgl  %s -spirv | FileCheck %s

// Make sure that:
// * [[vk::offset]] on an internal struct affects layout of an external struct.
// * [[vk::offset]] affects all variables of the struct type containing it.
// * We follow the normal rules for the fields without [[vk::offset]] specified.

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 32
// CHECK: OpMemberDecorate %S 2 Offset 64
// CHECK: OpMemberDecorate %S 3 Offset 80

// CHECK: OpMemberDecorate %type_ConstantBuffer_T 0 Offset 4
// CHECK: OpMemberDecorate %type_ConstantBuffer_T 1 Offset 16
// CHECK: OpMemberDecorate %type_ConstantBuffer_T 2 Offset 116

// CHECK: OpMemberDecorate %S_0 0 Offset 0
// CHECK: OpMemberDecorate %S_0 1 Offset 32
// CHECK: OpMemberDecorate %S_0 2 Offset 64
// CHECK: OpMemberDecorate %S_0 3 Offset 76

// CHECK: OpMemberDecorate %T 0 Offset 4
// CHECK: OpMemberDecorate %T 1 Offset 8
// CHECK: OpMemberDecorate %T 2 Offset 92

// CHECK: %type_ConstantBuffer_T = OpTypeStruct %float %S %float
// CHECK: %T = OpTypeStruct %float %S_0 %float

struct S {
    float  a;
    [[vk::offset(32)]]
    float2 b;
    [[vk::offset(64)]]
    float3 c;
    float  d[2];
};

struct T {
    [[vk::offset(4)]]
    float a;
    S     s;
    float b;
};

ConstantBuffer<T>   MyCBuffer;
StructuredBuffer<T> MySBuffer;

float4 main() : A {
    return MyCBuffer.s.a + MySBuffer[0].s.a;
}
