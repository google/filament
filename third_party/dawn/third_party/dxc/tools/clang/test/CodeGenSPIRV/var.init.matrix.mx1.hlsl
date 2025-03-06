// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

// CHECK:      [[cc00:%[0-9]+]] = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
// CHECK-NEXT: OpStore %mat1 [[cc00]]
    float3x1 mat1 = {1., 2., 3.};
// CHECK-NEXT: [[cc01:%[0-9]+]] = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
// CHECK-NEXT: OpStore %mat2 [[cc01]]
    float3x1 mat2 = {1., {2., {{3.}}}};
// CHECK-NEXT: [[cc02:%[0-9]+]] = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
// CHECK-NEXT: OpStore %mat3 [[cc02]]
    float3x1 mat3 = float3x1(1., 2., 3.);
// CHECK-NEXT: [[mat3:%[0-9]+]] = OpLoad %v3float %mat3
// CHECK-NEXT: OpStore %mat4 [[mat3]]
    float3x1 mat4 = float3x1(mat3);

    int scalar;
    bool1 vec1;
    uint2 vec2;
// CHECK-NEXT: [[scalar:%[0-9]+]] = OpLoad %int %scalar
// CHECK-NEXT: [[cv0:%[0-9]+]] = OpConvertSToF %float [[scalar]]
// CHECK-NEXT: [[vec2:%[0-9]+]] = OpLoad %v2uint %vec2
// CHECK-NEXT: [[vec1:%[0-9]+]] = OpLoad %bool %vec1
// CHECK-NEXT: [[ce0:%[0-9]+]] = OpCompositeExtract %uint [[vec2]] 0
// CHECK-NEXT: [[ce1:%[0-9]+]] = OpCompositeExtract %uint [[vec2]] 1
// CHECK-NEXT: [[cv1:%[0-9]+]] = OpConvertUToF %float [[ce0]]
// CHECK-NEXT: [[cv2:%[0-9]+]] = OpConvertUToF %float [[ce1]]
// CHECK-NEXT: [[cv3:%[0-9]+]] = OpSelect %float [[vec1]] %float_1 %float_0
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v4float [[cv0]] [[cv1]] [[cv2]] [[cv3]]
// CHECK-NEXT: OpStore %mat5 [[cc0]]
    float4x1 mat5 = {scalar, vec2, vec1};

    float2x1 mat6;
// CHECK-NEXT: [[mat6_0:%[0-9]+]] = OpLoad %v2float %mat6
// CHECK-NEXT: [[mat6_1:%[0-9]+]] = OpLoad %v2float %mat6
// CHECK-NEXT: [[ce2:%[0-9]+]] = OpCompositeExtract %float [[mat6_0]] 0
// CHECK-NEXT: [[ce3:%[0-9]+]] = OpCompositeExtract %float [[mat6_0]] 1
// CHECK-NEXT: [[ce4:%[0-9]+]] = OpCompositeExtract %float [[mat6_1]] 0
// CHECK-NEXT: [[ce5:%[0-9]+]] = OpCompositeExtract %float [[mat6_1]] 1
// CHECK-NEXT: [[cc1:%[0-9]+]] = OpCompositeConstruct %v4float [[ce2]] [[ce3]] [[ce4]] [[ce5]]
// CHECK-NEXT: OpStore %mat7 [[cc1]]
    float4x1 mat7 = {mat6, mat6};
}
