// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
    float2 foo[4] : SV_ClipDistance0;
    float3 bar[2] : SV_ClipDistance1;
};

float4 main(PSInput input) : SV_TARGET
{
// CHECK:  [[ptr0:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_0
// CHECK: [[load0:%[0-9]+]] = OpLoad %float [[ptr0]]

// CHECK:  [[ptr1:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_1
// CHECK: [[load1:%[0-9]+]] = OpLoad %float [[ptr1]]

// CHECK: [[vec0:%[0-9]+]] = OpCompositeConstruct %v2float [[load0]] [[load1]]

// CHECK:  [[ptr2:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_2
// CHECK: [[load2:%[0-9]+]] = OpLoad %float [[ptr2]]

// CHECK:  [[ptr3:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_3
// CHECK: [[load3:%[0-9]+]] = OpLoad %float [[ptr3]]

// CHECK: [[vec1:%[0-9]+]] = OpCompositeConstruct %v2float [[load2]] [[load3]]

// CHECK:  [[ptr4:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_4
// CHECK: [[load4:%[0-9]+]] = OpLoad %float [[ptr4]]

// CHECK:  [[ptr5:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_5
// CHECK: [[load5:%[0-9]+]] = OpLoad %float [[ptr5]]

// CHECK: [[vec2:%[0-9]+]] = OpCompositeConstruct %v2float [[load4]] [[load5]]

// CHECK:  [[ptr6:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_6
// CHECK: [[load6:%[0-9]+]] = OpLoad %float [[ptr6]]

// CHECK:  [[ptr7:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_7
// CHECK: [[load7:%[0-9]+]] = OpLoad %float [[ptr7]]

// CHECK:   [[vec3:%[0-9]+]] = OpCompositeConstruct %v2float [[load6]] [[load7]]
// CHECK: [[array0:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_4 [[vec0]] [[vec1]] [[vec2]] [[vec3]]

// CHECK:  [[ptr8:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_8
// CHECK: [[load8:%[0-9]+]] = OpLoad %float [[ptr8]]

// CHECK:  [[ptr9:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_9
// CHECK: [[load9:%[0-9]+]] = OpLoad %float [[ptr9]]

// CHECK:  [[ptr10:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_10
// CHECK: [[load10:%[0-9]+]] = OpLoad %float [[ptr10]]

// CHECK: [[vec0_0:%[0-9]+]] = OpCompositeConstruct %v3float [[load8]] [[load9]] [[load10]]

// CHECK:  [[ptr11:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_11
// CHECK: [[load11:%[0-9]+]] = OpLoad %float [[ptr11]]

// CHECK:  [[ptr12:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_12
// CHECK: [[load12:%[0-9]+]] = OpLoad %float [[ptr12]]

// CHECK:  [[ptr13:%[0-9]+]] = OpAccessChain %_ptr_Input_float %gl_ClipDistance %uint_13
// CHECK: [[load13:%[0-9]+]] = OpLoad %float [[ptr13]]

// CHECK:   [[vec1_0:%[0-9]+]] = OpCompositeConstruct %v3float [[load11]] [[load12]] [[load13]]
// CHECK: [[array1:%[0-9]+]] = OpCompositeConstruct %_arr_v3float_uint_2 [[vec0_0]] [[vec1_0]]

// CHECK:                   OpCompositeConstruct %PSInput [[array0]] [[array1]]

    float4 result = {input.foo[0], input.bar[1].yz};
    return result;
}
