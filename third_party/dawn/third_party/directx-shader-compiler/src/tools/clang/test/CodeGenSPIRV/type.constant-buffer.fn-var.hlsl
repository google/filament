// RUN: %dxc -T vs_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK-DAG: OpMemberDecorate %Inner 0 Offset 0

// CHECK-DAG:                   %Inner = OpTypeStruct %float
// CHECK-DAG: %type_ConstantBuffer_CBS = OpTypeStruct %Inner
// CHECK-DAG:                 %Inner_0 = OpTypeStruct %float
// CHECK-DAG:                     %CBS = OpTypeStruct %Inner_0
struct Inner
{
    float field;
};

struct CBS
{
    Inner entry;
};

ConstantBuffer<CBS> input;

float foo()
{
    ConstantBuffer<CBS> local = input;
    CBS alias = local;
// CHECK: %local = OpVariable %_ptr_Function_type_ConstantBuffer_CBS Function
// CHECK: [[loadedInput:%[0-9]+]] = OpLoad %type_ConstantBuffer_CBS %input
// CHECK: OpStore %local [[loadedInput]]
// CHECK: [[loadedLocal:%[0-9]+]] = OpLoad %type_ConstantBuffer_CBS %local
// CHECK:   [[e1:%[0-9]+]] = OpCompositeExtract %Inner [[loadedLocal]]
// CHECK:   [[e2:%[0-9]+]] = OpCompositeExtract %float [[e1]]
// CHECK:   [[c2:%[0-9]+]] = OpCompositeConstruct %Inner_0 [[e2]]
// CHECK:   [[c1:%[0-9]+]] = OpCompositeConstruct %CBS [[c2]]
// CHECK:                    OpStore %alias [[c1]]
    return alias.entry.field;
}

float main() : A
{
    return foo();
}
