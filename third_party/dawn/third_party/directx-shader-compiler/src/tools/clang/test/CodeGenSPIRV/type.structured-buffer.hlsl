// RUN: %dxc -T ps_6_0 -E main -fspv-reflect -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_GOOGLE_hlsl_functionality1"

// CHECK: OpName %type_StructuredBuffer_S "type.StructuredBuffer.S"
// CHECK: OpName %type_StructuredBuffer_T "type.StructuredBuffer.T"

// CHECK: OpName %type_RWStructuredBuffer_S "type.RWStructuredBuffer.S"
// CHECK: OpName %type_RWStructuredBuffer_T "type.RWStructuredBuffer.T"

// CHECK-NOT: OpDecorateId %mySBuffer3 CounterBuffer %counter_var_mySBuffer3
// CHECK-NOT: OpDecorateId %mySBuffer4 CounterBuffer %counter_var_mySBuffer4

// CHECK: %S = OpTypeStruct %float %v3float %mat2v3float
// CHECK: %_runtimearr_S = OpTypeRuntimeArray %S
// CHECK: %type_StructuredBuffer_S = OpTypeStruct %_runtimearr_S
// CHECK: %_ptr_Uniform_type_StructuredBuffer_S = OpTypePointer Uniform %type_StructuredBuffer_S
struct S {
    float    a;
    float3   b;
    float2x3 c;
};

// CHECK: %T = OpTypeStruct %_arr_float_uint_3 %_arr_v3float_uint_4 %_arr_S_uint_3 %_arr_mat3v2float_uint_4
// CHECK: %_runtimearr_T = OpTypeRuntimeArray %T
// CHECK: %type_StructuredBuffer_T = OpTypeStruct %_runtimearr_T
// CHECK: %_ptr_Uniform_type_StructuredBuffer_T = OpTypePointer Uniform %type_StructuredBuffer_T

// CHECK: %type_RWStructuredBuffer_S = OpTypeStruct %_runtimearr_S
// CHECK: %_ptr_Uniform_type_RWStructuredBuffer_S = OpTypePointer Uniform %type_RWStructuredBuffer_S

// CHECK: %type_RWStructuredBuffer_T = OpTypeStruct %_runtimearr_T
// CHECK: %_ptr_Uniform_type_RWStructuredBuffer_T = OpTypePointer Uniform %type_RWStructuredBuffer_T
struct T {
    float    a[3];
    float3   b[4];
    S        c[3];
    float3x2 d[4];
};

// CHECK: %mySBuffer1 = OpVariable %_ptr_Uniform_type_StructuredBuffer_S Uniform
StructuredBuffer<S> mySBuffer1 : register(t1);
// CHECK: %mySBuffer2 = OpVariable %_ptr_Uniform_type_StructuredBuffer_T Uniform
StructuredBuffer<T> mySBuffer2 : register(t2);

// CHECK: %mySBuffer3 = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_S Uniform
RWStructuredBuffer<S> mySBuffer3 : register(u3);
// CHECK: %mySBuffer4 = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_T Uniform
RWStructuredBuffer<T> mySBuffer4 : register(u4);

float4 main() : SV_Target {
    return 1.0;
}
