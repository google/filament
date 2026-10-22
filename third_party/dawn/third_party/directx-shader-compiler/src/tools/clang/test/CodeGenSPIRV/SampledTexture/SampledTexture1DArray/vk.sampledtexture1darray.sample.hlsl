// RUN: %dxc -T ps_6_7 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array
// CHECK: %type_1d_image_array_0 = OpTypeImage %uint 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image_0 = OpTypeSampledImage %type_1d_image_array_0

vk::SampledTexture1DArray<float4> tex1dArrayf4;
vk::SampledTexture1DArray<uint> tex1dArrayuint;
vk::SampledTexture1DArray tex1dArray_default;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArrayf4
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1_load]] [[sample_coord:%[a-zA-Z0-9_]+]] None
// CHECK: OpStore %val1 [[sampled_result]]

    float4 val1 = tex1dArrayf4.Sample(float2(0.5, 0.25));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray_default
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex2_load]] [[sample_coord_2:%[a-zA-Z0-9_]+]] ConstOffset %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]

    float4 val2 = tex1dArray_default.Sample(float2(0.5, 0.25), 2);

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArrayf4
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex3_load]] [[sample_coord_3:%[a-zA-Z0-9_]+]] ConstOffset|MinLod %int_2 %float_1
// CHECK: OpStore %val3 [[sampled_result_3]]

    float4 val3 = tex1dArrayf4.Sample(float2(0.5, 0.25), 2, 1.0f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArrayf4
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex4_load]] [[sample_coord_4:%[a-zA-Z0-9_]+]] ConstOffset|MinLod %int_2 %float_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]

    uint status;
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]

    float4 val4 = tex1dArrayf4.Sample(float2(0.5, 0.25), 2, 1.0f, status);

// CHECK: [[tex5_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image_0 %tex1dArrayuint
// CHECK: [[sampled_result_5:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4uint [[tex5_load]] [[sample_coord_5:%[a-zA-Z0-9_]+]] None
// CHECK: [[status_0_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_5]] 0
// CHECK: OpStore %val5 [[status_0_2]]

    uint val5 = tex1dArrayuint.Sample(float2(0.5, 0.25));

    return 1.0;
}
