// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct VS_OUTPUT {
    float4 clips[2] : SV_ClipDistance;
};

// CHECK:       %VS_OUTPUT = OpTypeStruct %_arr_v4float_uint_2
// CHECK: %gl_ClipDistance = OpVariable %_ptr_Output__arr_float_uint_8 Output

VS_OUTPUT main() {
// CHECK: [[VS_OUTPUT:%[0-9]+]] = OpFunctionCall %VS_OUTPUT %src_main

// CHECK:  [[clips:%[0-9]+]] = OpCompositeExtract %_arr_v4float_uint_2 [[VS_OUTPUT]] 0
// CHECK: [[offset_base:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_0

// CHECK: [[offset:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_0
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset]]
// CHECK:  [[value:%[0-9]+]] = OpCompositeExtract %float [[clips]] 0 0
// CHECK:                   OpStore [[ptr]] [[value]]

// CHECK: [[offset_0:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_1
// CHECK:    [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_0]]
// CHECK:  [[value_0:%[0-9]+]] = OpCompositeExtract %float [[clips]] 0 1
// CHECK:                   OpStore [[ptr_0]] [[value_0]]

// CHECK: [[offset_1:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_2
// CHECK:    [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_1]]
// CHECK:  [[value_1:%[0-9]+]] = OpCompositeExtract %float [[clips]] 0 2
// CHECK:                   OpStore [[ptr_1]] [[value_1]]

// CHECK: [[offset_2:%[0-9]+]] = OpIAdd %uint [[offset_base]] %uint_3
// CHECK:    [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_2]]
// CHECK:  [[value_2:%[0-9]+]] = OpCompositeExtract %float [[clips]] 0 3
// CHECK:                   OpStore [[ptr_2]] [[value_2]]

// CHECK: [[offset_base_0:%[0-9]+]] = OpIAdd %uint %uint_0 %uint_4

// CHECK: [[offset_3:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_0
// CHECK:    [[ptr_3:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_3]]
// CHECK:  [[value_3:%[0-9]+]] = OpCompositeExtract %float [[clips]] 1 0
// CHECK:                   OpStore [[ptr_3]] [[value_3]]

// CHECK: [[offset_4:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_1
// CHECK:    [[ptr_4:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_4]]
// CHECK:  [[value_4:%[0-9]+]] = OpCompositeExtract %float [[clips]] 1 1
// CHECK:                   OpStore [[ptr_4]] [[value_4]]

// CHECK: [[offset_5:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_2
// CHECK:    [[ptr_5:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_5]]
// CHECK:  [[value_5:%[0-9]+]] = OpCompositeExtract %float [[clips]] 1 2
// CHECK:                   OpStore [[ptr_5]] [[value_5]]

// CHECK: [[offset_6:%[0-9]+]] = OpIAdd %uint [[offset_base_0]] %uint_3
// CHECK:    [[ptr_6:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_ClipDistance [[offset_6]]
// CHECK:  [[value_6:%[0-9]+]] = OpCompositeExtract %float [[clips]] 1 3
// CHECK:                   OpStore [[ptr_6]] [[value_6]]
    return (VS_OUTPUT) 0;
}
