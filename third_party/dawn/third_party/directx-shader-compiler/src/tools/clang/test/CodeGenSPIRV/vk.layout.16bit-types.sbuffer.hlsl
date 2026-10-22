// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// XXXXX: OpCapability StorageBuffer16BitAccess

// CHECK: OpExtension "SPV_KHR_16bit_storage"

// CHECK: OpMemberDecorate %S 0 Offset 0
// CHECK: OpMemberDecorate %S 0 MatrixStride 8
// CHECK: OpMemberDecorate %S 0 RowMajor
// CHECK: OpMemberDecorate %S 1 Offset 16
// CHECK: OpMemberDecorate %S 2 Offset 20

// CHECK: OpMemberDecorate %T 0 Offset 0
// CHECK: OpMemberDecorate %T 1 Offset 32

// CHECK: OpDecorate %_runtimearr_T ArrayStride 48

// CHECK: OpMemberDecorate %type_StructuredBuffer_T 0 Offset 0
// CHECK: OpMemberDecorate %type_StructuredBuffer_T 0 NonWritable

// CHECK: OpDecorate %type_StructuredBuffer_T BufferBlock

struct S {
    float16_t3x2 val1; // Nested 16-bit matrix
    uint16_t2    val2; // Nested 16-bit vector
    int16_t      val3; // Nested 16-bit scalar

};

struct T {
    S      nested;
    float4 val;
};

StructuredBuffer<T> MySBuffer;

float4 main() : SV_Target {
    return MySBuffer[0].val;
}
