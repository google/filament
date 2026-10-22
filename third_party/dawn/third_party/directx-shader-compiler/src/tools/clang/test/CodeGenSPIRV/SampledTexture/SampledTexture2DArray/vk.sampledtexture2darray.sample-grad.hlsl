// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_1
// CHECK: [[v2f_1:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_1
// CHECK: [[v2f_2:%[0-9]+]] = OpConstantComposite %v2float %float_2 %float_2
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex1_load]] [[v2fc]] Grad [[v2f_1]] [[v2f_2]]
    float4 val1 = tex2darray.SampleGrad(float3(0.5, 0.25, 0.1), float2(1, 1), float2(2, 2));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex2_load]] [[v2fc]] Grad|ConstOffset [[v2f_1]] [[v2f_2]] [[v2ic]]
    float4 val2 = tex2darray.SampleGrad(float3(0.5, 0.25, 0.1), float2(1, 1), float2(2, 2), int2(2,3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex3_load]] [[v2fc]] Grad|ConstOffset|MinLod [[v2f_1]] [[v2f_2]] [[v2ic]] %float_0_5
    float4 val3 = tex2darray.SampleGrad(float3(0.5, 0.25, 0.1), float2(1, 1), float2(2, 2), int2(2,3), 0.5);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[tex4_load]] [[v2fc]] Grad|ConstOffset|MinLod [[v2f_1]] [[v2f_2]] [[v2ic]] %float_0_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK:                        OpStore %status [[status_0]]
    uint status;
    float4 val4 = tex2darray.SampleGrad(float3(0.5, 0.25, 0.1), float2(1, 1), float2(2, 2), int2(2,3), 0.5, status);

    return 1.0;
}
