// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1_load]] [[sample_coord:%[a-zA-Z0-9_]+]] Bias %float_0_5
// CHECK: OpStore %val1 [[sampled_result]]

    float4 val1 = tex1dArray.SampleBias(float2(0.5, 0.25), 0.5f);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex2_load]] [[sample_coord_2:%[a-zA-Z0-9_]+]] Bias|ConstOffset %float_0_5 %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]

    float4 val2 = tex1dArray.SampleBias(float2(0.5, 0.25), 0.5f, 2);

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex3_load]] [[sample_coord_3:%[a-zA-Z0-9_]+]] Bias|ConstOffset|MinLod %float_0_5 %int_2 %float_2_5
// CHECK: OpStore %val3 [[sampled_result_3]]

    float4 val3 = tex1dArray.SampleBias(float2(0.5, 0.25), 0.5f, 2, 2.5f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex4_load]] [[sample_coord_4:%[a-zA-Z0-9_]+]] Bias|ConstOffset|MinLod %float_0_5 %int_2 %float_2_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]

    uint status;
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]

    float4 val4 = tex1dArray.SampleBias(float2(0.5, 0.25), 0.5f, 2, 2.5f, status);

    return 1.0;
}
