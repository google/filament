// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability StorageBuffer16BitAccess

// CHECK: OpExtension "SPV_KHR_16bit_storage"

// CHECK: OpMemberDecorate %type_MyTBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_MyTBuffer 0 NonWritable
// CHECK: OpMemberDecorate %type_MyTBuffer 1 Offset 4
// CHECK: OpMemberDecorate %type_MyTBuffer 1 NonWritable
// CHECK: OpMemberDecorate %type_MyTBuffer 2 Offset 8
// CHECK: OpMemberDecorate %type_MyTBuffer 2 MatrixStride 8
// CHECK: OpMemberDecorate %type_MyTBuffer 2 RowMajor
// CHECK: OpMemberDecorate %type_MyTBuffer 2 NonWritable
// CHECK: OpDecorate %type_MyTBuffer BufferBlock

tbuffer MyTBuffer {
    uint16_t2 gVal1; // 16-bit vector
    int16_t   gVal2; // 16-bit scalar
    half3x2   gVal3; // 16-bit matrix
};

float4 main() : SV_Target {
    return gVal2;
}
