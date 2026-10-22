// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex1_load]] %float_0_5 %float_1 Bias %float_0_5
// CHECK: OpStore %val1 [[sampled_result]]
    float val1 = tex1d.SampleCmpBias(0.5, 1.0f, 0.5f);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex2_load]] %float_0_5 %float_1 Bias|ConstOffset %float_0_5 %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]
    float val2 = tex1d.SampleCmpBias(0.5, 1.0f, 0.5f, 2);

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex3_load]] %float_0_5 %float_1 Bias|ConstOffset|MinLod %float_0_5 %int_2 %float_2_5
// CHECK: OpStore %val3 [[sampled_result_3]]
    float val3 = tex1d.SampleCmpBias(0.5, 1.0f, 0.5f, 2, 2.5f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[tex4_load]] %float_0_5 %float_1 Bias|ConstOffset|MinLod %float_0_5 %int_2 %float_2_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]
// CHECK: [[sampled_value:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_value]]
    uint status;
    float val4 = tex1d.SampleCmpBias(0.5, 1.0f, 0.5f, 2, 2.5f, status);

    return 1.0;
}
