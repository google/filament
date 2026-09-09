// RUN: %dxc -T vs_6_0 -E main -fvk-use-gl-layout -fcgl %s -spirv | FileCheck %s

// CHECK-DAG:  OpDecorate %_arr_float_uint_2 ArrayStride 4
// CHECK-DAG:  %type_TextureBuffer_CBS = OpTypeStruct %_arr_float_uint_2
// CHECK-DAG:  %CBS = OpTypeStruct %_arr_float_uint_2_0

struct CBS
{
    float entry[2];
};

float foo(TextureBuffer<CBS> param)
{
    CBS alias = param;
// CHECK: %param = OpFunctionParameter %_ptr_Function_type_TextureBuffer_CBS
// CHECK: [[copy:%[0-9]+]] = OpLoad %type_TextureBuffer_CBS %param
// CHECK:   [[e1:%[0-9]+]] = OpCompositeExtract %_arr_float_uint_2 [[copy]]
// CHECK:   [[e2:%[0-9]+]] = OpCompositeExtract %float [[e1]] 0
// CHECK:   [[e3:%[0-9]+]] = OpCompositeExtract %float [[e1]] 1
// CHECK:   [[c1:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_2_0 [[e2]] [[e3]]
// CHECK:   [[c2:%[0-9]+]] = OpCompositeConstruct %CBS [[c1]]
// CHECK:                    OpStore %alias [[c2]]
    return alias.entry[1];
}

TextureBuffer<CBS> input;

float main() : A
{
    return foo(input);
}

