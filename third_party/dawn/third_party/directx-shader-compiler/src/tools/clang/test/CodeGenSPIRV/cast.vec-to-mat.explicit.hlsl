// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float4 main(float4 input : A) : SV_Target {
// CHECK:       [[vec:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT: [[vec1:%[0-9]+]] = OpVectorShuffle %v2float [[vec]] [[vec]] 0 1
// CHECK-NEXT: [[vec2:%[0-9]+]] = OpVectorShuffle %v2float [[vec]] [[vec]] 2 3
// CHECK-NEXT:  [[mat:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[vec1]] [[vec2]]
// CHECK-NEXT:                 OpStore %mat1 [[mat]]
    float2x2 mat1 = (             float2x2)input;

// CHECK:       [[vec_0:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT: [[vec1_0:%[0-9]+]] = OpVectorShuffle %v2float [[vec_0]] [[vec_0]] 0 1
// CHECK-NEXT: [[vec2_0:%[0-9]+]] = OpVectorShuffle %v2float [[vec_0]] [[vec_0]] 2 3
// CHECK-NEXT:  [[mat_0:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[vec1_0]] [[vec2_0]]
// CHECK-NEXT:                 OpStore %mat2 [[mat_0]]
    float2x2 mat2 = (row_major    float2x2)input;

// CHECK:       [[vec_1:%[0-9]+]] = OpLoad %v4float %input
// CHECK-NEXT: [[vec1_1:%[0-9]+]] = OpVectorShuffle %v2float [[vec_1]] [[vec_1]] 0 1
// CHECK-NEXT: [[vec2_1:%[0-9]+]] = OpVectorShuffle %v2float [[vec_1]] [[vec_1]] 2 3
// CHECK-NEXT:  [[mat_1:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[vec1_1]] [[vec2_1]]
// CHECK-NEXT:                 OpStore %mat3 [[mat_1]]
    float2x2 mat3 = (column_major float2x2)input;

// CHECK:         [[a:%[0-9]+]] = OpLoad %v4int %a
// CHECK-NEXT: [[vec1_2:%[0-9]+]] = OpVectorShuffle %v2int [[a]] [[a]] 0 1
// CHECK-NEXT: [[vec2_2:%[0-9]+]] = OpVectorShuffle %v2int [[a]] [[a]] 2 3
// CHECK-NEXT:      {{%[0-9]+}} = OpCompositeConstruct %_arr_v2int_uint_2 [[vec1_2]] [[vec2_2]]
    int4 a;
    int2x2 b = a;

    return float4(mat1[0][0], mat2[0][1], mat3[1][0], mat1[1][1]);
}
