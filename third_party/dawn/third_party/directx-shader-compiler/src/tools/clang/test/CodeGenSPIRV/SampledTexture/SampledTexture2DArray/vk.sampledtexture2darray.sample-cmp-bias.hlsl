// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex1_load]] [[v2fc]] %float_1 Bias %float_0_5
    float val1 = tex2darray.SampleCmpBias(float3(0.5, 0.25, 0.1), 1.0f, 0.5f);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex2_load]] [[v2fc]] %float_1 Bias|ConstOffset %float_0_5 [[v2ic]]
    float val2 = tex2darray.SampleCmpBias(float3(0.5, 0.25, 0.1), 1.0f, 0.5f, int2(2, 3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex3_load]] [[v2fc]] %float_1 Bias|ConstOffset|MinLod %float_0_5 [[v2ic]] %float_2_5
    float val3 = tex2darray.SampleCmpBias(float3(0.5, 0.25, 0.1), 1.0f, 0.5f, int2(2, 3), 2.5f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[tex4_load]] [[v2fc]] %float_1 Bias|ConstOffset|MinLod %float_0_5 [[v2ic]] %float_2_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK:                        OpStore %status [[status_0]]
    uint status;
    float val4 = tex2darray.SampleCmpBias(float3(0.5, 0.25, 0.1), 1.0f, 0.5f, int2(2, 3), 2.5f, status);

    return 1.0;
}
