// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability UniformAndStorageBuffer16BitAccess
// CHECK: OpCapability Int16
// CHECK: OpCapability Float16

// CHECK: OpExtension "SPV_KHR_16bit_storage"

// CHECK: OpMemberDecorate %type_MyCBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyCBuffer 1 Offset 4
// CHECK: OpMemberDecorate %type_MyCBuffer 2 Offset 16
// CHECK: OpMemberDecorate %type_MyCBuffer 2 MatrixStride 16
// CHECK: OpMemberDecorate %type_MyCBuffer 2 RowMajor
// CHECK: OpDecorate %type_MyCBuffer Block

cbuffer MyCBuffer {
    uint16_t2 gVal1; // 16-bit vector
    int16_t   gVal2; // 16-bit scalar
    half3x2   gVal3; // 16-bit matrix
};

float4 main() : SV_Target {
    return gVal2;
}
