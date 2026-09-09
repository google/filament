// RUN: %dxc -T ps_6_7 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]
// CHECK: [[type_2d_image_array_uint:%[a-zA-Z0-9_]+]] = OpTypeImage %uint 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array_uint:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array_uint]]

vk::SampledTexture2DArray<float4> tex2dArrayf4;
vk::SampledTexture2DArray<uint> tex2dArrayuint;
vk::SampledTexture2DArray tex2dArray_default;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArrayf4
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1_load]] [[v2fc]] None
    float4 val1 = tex2dArrayf4.Sample(float3(0.5, 0.25, 0.1));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArray_default
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex2_load]] [[v2fc]] ConstOffset [[v2ic]]
    float4 val2 = tex2dArray_default.Sample(float3(0.5, 0.25, 0.1), int2(2, 3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArrayf4
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex3_load]] [[v2fc]] ConstOffset|MinLod [[v2ic]] %float_1
    float4 val3 = tex2dArrayf4.Sample(float3(0.5, 0.25, 0.1), int2(2, 3), 1.0f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArrayf4
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex4_load]] [[v2fc]] ConstOffset|MinLod [[v2ic]] %float_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK:                        OpStore %status [[status_0]]
    uint status;
    float4 val4 = tex2dArrayf4.Sample(float3(0.5, 0.25, 0.1), int2(2, 3), 1.0f, status);

// CHECK: [[tex5_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array_uint]] %tex2dArrayuint
// CHECK: [[sampled_result5:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4uint [[tex5_load]] [[v2fc]] None
// CHECK: [[val5:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result5]] 0
    uint val5 = tex2dArrayuint.Sample(float3(0.5, 0.25, 0.1));

    return 1.0;
}
