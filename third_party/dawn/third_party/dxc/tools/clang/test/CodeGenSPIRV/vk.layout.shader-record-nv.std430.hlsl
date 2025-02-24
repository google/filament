// RUN: %dxc -T lib_6_3 -fspv-extension=SPV_NV_ray_tracing -fvk-use-gl-layout -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr_v2float_uint_3 ArrayStride 8
// CHECK: OpDecorate %_arr_mat3v2float_uint_2 ArrayStride 32
// CHECK: OpDecorate %_arr_v2int_uint_3 ArrayStride 8
// CHECK: OpDecorate %_arr__arr_v2int_uint_3_uint_2 ArrayStride 24
// CHECK: OpDecorate %_arr_mat3v2float_uint_2_0 ArrayStride 24
// CHECK-NOT: OpDecorate %cbuf DescriptorSet
// CHECK-NOT: OpDecorate %cbuf Binding
// CHECK-NOT: OpDecorate %block DescriptorSet
// CHECK-NOT: OpDecorate %block Binding

// CHECK: OpMemberDecorate %T 0 Offset 0
// CHECK: OpMemberDecorate %T 1 Offset 32
// CHECK: OpMemberDecorate %T 1 MatrixStride 16
// CHECK: OpMemberDecorate %T 1 RowMajor
// CHECK: OpMemberDecorate %T 2 Offset 96
// CHECK: OpMemberDecorate %T 3 Offset 144
// CHECK: OpMemberDecorate %T 3 MatrixStride 8
// CHECK: OpMemberDecorate %T 3 ColMajor
struct T {
                 float2   f1[3];
    column_major float3x2 f2[2];
    row_major    int3x2   f4[2];
    row_major    float3x2 f3[2];
};

// CHECK: OpMemberDecorate %type_ConstantBuffer_S 0 Offset 0
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 1 Offset 16
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 2 Offset 32
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 3 Offset 224
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 4 Offset 256
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 4 MatrixStride 16
// CHECK: OpMemberDecorate %type_ConstantBuffer_S 4 ColMajor

struct S {
              float    f1;
              float3   f2;
              T        f4;
    row_major int2x3   f5;
    row_major float2x3 f3;
};

[[vk::shader_record_nv]]
ConstantBuffer<S> cbuf;

// CHECK: OpDecorate %type_ConstantBuffer_S Block
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 0 Offset 0
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 1 Offset 16
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 2 Offset 32
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 3 Offset 224
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 4 Offset 256
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 4 MatrixStride 16
// CHECK: OpMemberDecorate %type_ShaderRecordBufferNV_block 4 ColMajor


[[vk::shader_record_nv]]
cbuffer block {
              float    f1;
              float3   f2;
              T        f4;
    row_major int2x3   f5;
    row_major float2x3 f3;
}

// CHECK: OpDecorate %type_ShaderRecordBufferNV_block Block
struct Payload { float p; };
struct Attr    { float a; };

// CHECK: %_ptr_ShaderRecordBufferKHR_type_ConstantBuffer_S = OpTypePointer ShaderRecordBufferKHR %type_ConstantBuffer_S
// CHECK: %cbuf = OpVariable %_ptr_ShaderRecordBufferKHR_type_ConstantBuffer_S ShaderRecordBufferKHR

[shader("closesthit")]
void chs1(inout Payload P, in Attr A) {
    P.p = cbuf.f1;
}

[shader("closesthit")]
void chs2(inout Payload P, in Attr A) {
    P.p = f1;
}
