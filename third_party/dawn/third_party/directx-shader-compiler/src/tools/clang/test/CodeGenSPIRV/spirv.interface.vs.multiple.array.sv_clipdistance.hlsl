// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct VS_OUTPUT {
    float2 foo[4] : SV_ClipDistance0;
    float3 bar[2] : SV_ClipDistance1;
};

// CHECK:       %VS_OUTPUT = OpTypeStruct %_arr_v2float_uint_4 %_arr_v3float_uint_2
// CHECK: %gl_ClipDistance = OpVariable %_ptr_Output__arr_float_uint_14 Output

VS_OUTPUT main() {
// CHECK: [[VS_OUTPUT:%[0-9]+]] = OpFunctionCall %VS_OUTPUT %src_main

// CHECK:  [[foo:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_4 [[VS_OUTPUT]] 0
// CHECK: [[offset_base:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_0

// CHECK: [[offset:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_0
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset]]
// CHECK:  [[value:%[0-9]+]] = OpCompositeExtract %float [[foo]] 0 0
// CHECK:                   OpStore [[ptr]] [[value]]

// CHECK: [[offset_0:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_1
// CHECK:    [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_0]]
// CHECK:  [[value_0:%[0-9]+]] = OpCompositeExtract %float [[foo]] 0 1
// CHECK:                   OpStore [[ptr_0]] [[value_0]]

// CHECK: [[offset_base_0:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_2

// CHECK: [[offset_1:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_0
// CHECK:    [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_1]]
// CHECK:  [[value_1:%[0-9]+]] = OpCompositeExtract %float [[foo]] 1 0
// CHECK:                   OpStore [[ptr_1]] [[value_1]]

// CHECK: [[offset_2:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_1
// CHECK:    [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_2]]
// CHECK:  [[value_2:%[0-9]+]] = OpCompositeExtract %float [[foo]] 1 1
// CHECK:                   OpStore [[ptr_2]] [[value_2]]

// CHECK: [[offset_base_1:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_4

// CHECK: [[offset_3:%[0-9]+]] = OpIAdd %uint [[offset_base_1]] %uint_0
// CHECK:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_3]]
// CHECK:  [[value_3:%[0-9]+]] = OpCompositeExtract %float [[foo]] 2 0
// CHECK:                   OpStore [[ptr_3]] [[value_3]]

// CHECK: [[offset_4:%[0-9]+]] = OpIAdd %uint [[offset_base_1]] %uint_1
// CHECK:    [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_4]]
// CHECK:  [[value_4:%[0-9]+]] = OpCompositeExtract %float [[foo]] 2 1
// CHECK:                   OpStore [[ptr_4]] [[value_4]]

// CHECK: [[offset_base_2:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_6

// CHECK: [[offset_5:%[0-9]+]] = OpIAdd %uint [[offset_base_2]] %uint_0
// CHECK:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_5]]
// CHECK:  [[value_5:%[0-9]+]] = OpCompositeExtract %float [[foo]] 3 0
// CHECK:                   OpStore [[ptr_5]] [[value_5]]

// CHECK: [[offset_6:%[0-9]+]] = OpIAdd %uint [[offset_base_2]] %uint_1
// CHECK:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_6]]
// CHECK:  [[value_6:%[0-9]+]] = OpCompositeExtract %float [[foo]] 3 1
// CHECK:                   OpStore [[ptr_6]] [[value_6]]

// CHECK:  [[bar:%[0-9]+]] = OpCompositeExtract %_arr_v3float_uint_2 [[VS_OUTPUT]] 1
// CHECK: [[offset_base_3:%[0-9]+]] = OpIAdd %uint %uint_8 %uint_0

// CHECK: [[offset_7:%[0-9]+]] = OpIAdd %uint [[offset_base_3]] %uint_0
// CHECK:    [[ptr_7:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_7]]
// CHECK:  [[value_7:%[0-9]+]] = OpCompositeExtract %float [[bar]] 0 0
// CHECK:                   OpStore [[ptr_7]] [[value_7]]

// CHECK: [[offset_8:%[0-9]+]] = OpIAdd %uint [[offset_base_3]] %uint_1
// CHECK:    [[ptr_8:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_8]]
// CHECK:  [[value_8:%[0-9]+]] = OpCompositeExtract %float [[bar]] 0 1
// CHECK:                   OpStore [[ptr_8]] [[value_8]]

// CHECK: [[offset_9:%[0-9]+]] = OpIAdd %uint [[offset_base_3]] %uint_2
// CHECK:    [[ptr_9:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_9]]
// CHECK:  [[value_9:%[0-9]+]] = OpCompositeExtract %float [[bar]] 0 2

// CHECK: [[offset_base_4:%[0-9]+]] = OpIAdd %uint %uint_8 %uint_3

// CHECK: [[offset_10:%[0-9]+]] = OpIAdd %uint [[offset_base_4]] %uint_0
// CHECK:    [[ptr_10:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_10]]
// CHECK:  [[value_10:%[0-9]+]] = OpCompositeExtract %float [[bar]] 1 0
// CHECK:                   OpStore [[ptr_10]] [[value_10]]

// CHECK: [[offset_11:%[0-9]+]] = OpIAdd %uint [[offset_base_4]] %uint_1
// CHECK:    [[ptr_11:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_11]]
// CHECK:  [[value_11:%[0-9]+]] = OpCompositeExtract %float [[bar]] 1 1
// CHECK:                   OpStore [[ptr_11]] [[value_11]]

// CHECK: [[offset_12:%[0-9]+]] = OpIAdd %uint [[offset_base_4]] %uint_2
// CHECK:    [[ptr_12:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_12]]
// CHECK:  [[value_12:%[0-9]+]] = OpCompositeExtract %float [[bar]] 1 2

    return (VS_OUTPUT) 0;
}
