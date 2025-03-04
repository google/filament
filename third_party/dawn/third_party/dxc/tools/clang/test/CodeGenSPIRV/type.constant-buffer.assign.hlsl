// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpName %type_ConstantBuffer_S "type.ConstantBuffer.S"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_S 0 "someFloat"
// CHECK-NEXT: OpMemberName %type_ConstantBuffer_S 1 "another"

// CHECK:      OpDecorate %type_ConstantBuffer_S Block

// CHECK: %type_ConstantBuffer_S = OpTypeStruct %float %float

struct S
{
    float someFloat;
    float another;
};

// CHECK: %_ptr_Uniform_type_ConstantBuffer_S = OpTypePointer Uniform %type_ConstantBuffer_S
// CHECK: %_ptr_Function_type_ConstantBuffer_S = OpTypePointer Function %type_ConstantBuffer_S

// CHECK: %buffer = OpVariable %_ptr_Uniform_type_ConstantBuffer_S Uniform
ConstantBuffer<S> buffer;

void main()
{
// CHECK:           %local = OpVariable %_ptr_Function_type_ConstantBuffer_S Function

// CHECK: [[val:%[0-9]+]] = OpLoad %type_ConstantBuffer_S %buffer
// CHECK:                OpStore %local [[val]]
  ConstantBuffer<S> local;
  local = buffer;
}
