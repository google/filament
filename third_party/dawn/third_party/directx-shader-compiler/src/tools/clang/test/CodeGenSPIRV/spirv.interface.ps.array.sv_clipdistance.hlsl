// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
    float4 color[2] : SV_ClipDistance;
};

float4 main(PSInput input) : SV_TARGET
{
// CHECK:  [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0
// CHECK: [[load0:%[0-9]+]] = OpLoad %float [[ptr0]]

// CHECK:  [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_1
// CHECK: [[load1:%[0-9]+]] = OpLoad %float [[ptr1]]

// CHECK:  [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_2
// CHECK: [[load2:%[0-9]+]] = OpLoad %float [[ptr2]]

// CHECK:  [[ptr3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_3
// CHECK: [[load3:%[0-9]+]] = OpLoad %float [[ptr3]]

// CHECK: [[vec0:%[0-9]+]] = OpCompositeConstruct %v4float [[load0]] [[load1]] [[load2]] [[load3]]

// CHECK:  [[ptr4:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_4
// CHECK: [[load4:%[0-9]+]] = OpLoad %float [[ptr4]]

// CHECK:  [[ptr5:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_5
// CHECK: [[load5:%[0-9]+]] = OpLoad %float [[ptr5]]

// CHECK:  [[ptr6:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_6
// CHECK: [[load6:%[0-9]+]] = OpLoad %float [[ptr6]]

// CHECK:  [[ptr7:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_7
// CHECK: [[load7:%[0-9]+]] = OpLoad %float [[ptr7]]

// CHECK: [[vec1:%[0-9]+]] = OpCompositeConstruct %v4float [[load4]] [[load5]] [[load6]] [[load7]]

// CHECK: [[array:%[0-9]+]] = OpCompositeConstruct %_arr_v4float_uint_2 [[vec0]] [[vec1]]
// CHECK:                  OpCompositeConstruct %PSInput [[array]]

    return input.color[0];
}
