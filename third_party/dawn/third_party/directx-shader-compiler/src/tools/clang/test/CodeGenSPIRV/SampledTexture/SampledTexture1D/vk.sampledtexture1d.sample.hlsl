// RUN: %dxc -T ps_6_7 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image
// CHECK: %type_1d_image_0 = OpTypeImage %uint 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image_0 = OpTypeSampledImage %type_1d_image_0

vk::SampledTexture1D<float4> tex1df4;
vk::SampledTexture1D<uint> tex1duint;
vk::SampledTexture1D tex1d_default;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1df4
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1_load]] %float_0_5 None
// CHECK: OpStore %val1 [[sampled_result]]
    float4 val1 = tex1df4.Sample(0.5);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d_default
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex2_load]] %float_0_5 ConstOffset %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]
    float4 val2 = tex1d_default.Sample(0.5, 2);

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1df4
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex3_load]] %float_0_5 ConstOffset|MinLod %int_2 %float_1
// CHECK: OpStore %val3 [[sampled_result_3]]
    float4 val3 = tex1df4.Sample(0.5, 2, 1.0f);

    uint status;
// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1df4
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex4_load]] %float_0_5 ConstOffset|MinLod %int_2 %float_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]
    float4 val4 = tex1df4.Sample(0.5, 2, 1.0f, status);

// CHECK: [[tex5_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image_0 %tex1duint
// CHECK: [[sampled_result_5:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4uint [[tex5_load]] %float_0_5 None
// CHECK: [[status_0_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_5]] 0
// CHECK: OpStore %val5 [[status_0_2]]

    uint val5 = tex1duint.Sample(0.5);

    return 1.0;
}
