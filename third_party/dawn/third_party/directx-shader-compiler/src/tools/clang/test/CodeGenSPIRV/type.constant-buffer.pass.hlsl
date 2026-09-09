// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

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

float foo(ConstantBuffer<CBS> param)
{
    CBS alias = param;
// CHECK: [[copy:%[0-9]+]] = OpLoad %type_ConstantBuffer_CBS %param
// CHECK:   [[e1:%[0-9]+]] = OpCompositeExtract %Inner [[copy]]
// CHECK:   [[e2:%[0-9]+]] = OpCompositeExtract %float [[e1]]
// CHECK:   [[c2:%[0-9]+]] = OpCompositeConstruct %Inner_0 [[e2]]
// CHECK:   [[c1:%[0-9]+]] = OpCompositeConstruct %CBS [[c2]]
// CHECK:                    OpStore %alias [[c1]]
    return alias.entry.field;
}

ConstantBuffer<CBS> input;

float main() : A
{
    return foo(input);
}
