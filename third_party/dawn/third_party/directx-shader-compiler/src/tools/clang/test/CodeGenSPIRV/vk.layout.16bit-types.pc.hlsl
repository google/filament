// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability StoragePushConstant16

// CHECK: OpExtension "SPV_KHR_16bit_storage"

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 1 Offset 2
// CHECK: OpMemberDecorate %S 2 Offset 8
// CHECK: OpMemberDecorate %S 2 MatrixStride 4
// CHECK: OpMemberDecorate %S 2 RowMajor
// CHECK: OpMemberDecorate %type_PushConstant_T 0 Offset 0
// CHECK: OpMemberDecorate %type_PushConstant_T 1 Offset 32
// CHECK: OpDecorate %type_PushConstant_T Block

struct S {
    int16_t      val1; // Nested 16-bit scalar
    uint16_t2    val2; // Nested 16-bit vector
    float16_t2x3 val3; // Nested 16-bit matrix

};

struct T {
    S      nested;
    float4 val;
};

[[vk::push_constant]]
T MyPC;

float4 main() : SV_Target {
    return MyPC.val;
}
