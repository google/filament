// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %type_ConstantBuffer_T "type.ConstantBuffer.T"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 0 "a"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 1 "b"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 2 "c"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 3 "d"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 4 "s"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_T 5 "t"

// CHECK:      OpName %MyCbuffer "MyCbuffer"
// CHECK:      OpName %AnotherCBuffer "AnotherCBuffer"

// CHECK:      OpDecorate %type_ConstantBuffer_T Block

struct S {
    float  f1;
    float3 f2;
};

// CHECK: %type_ConstantBuffer_T = OpTypeStruct %uint %int %v2uint %mat3v4float %S %_arr_float_uint_4
struct T {
    bool     a;
    int      b;
    uint2    c;
    float3x4 d;
    S        s;
    float    t[4];
};

// CHECK: %_ptr_Uniform_type_ConstantBuffer_T = OpTypePointer Uniform %type_ConstantBuffer_T
// CHECK: %_arr_type_ConstantBuffer_T_uint_2 = OpTypeArray %type_ConstantBuffer_T %uint_2
// CHECK: %_ptr_Uniform__arr_type_ConstantBuffer_T_uint_2 = OpTypePointer Uniform %_arr_type_ConstantBuffer_T_uint_2

// CHECK: %MyCbuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_T Uniform
ConstantBuffer<T> MyCbuffer : register(b1);
// CHECK: %AnotherCBuffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_T Uniform
ConstantBuffer<T> AnotherCBuffer : register(b2);
// CHECK: %MyConstantBufferArray = OpVariable %_ptr_Uniform__arr_type_ConstantBuffer_T_uint_2 Uniform
ConstantBuffer<T> MyConstantBufferArray[2] : register(b3);

void main() {
}
