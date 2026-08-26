// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex1_load]] %float_0_5 Grad %float_1 %float_2
// CHECK: OpStore %val1 [[sampled_result]]
    float4 val1 = tex1d.SampleGrad(0.5, 1, 2);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex2_load]] %float_0_5 Grad|ConstOffset %float_1 %float_2 %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]
    float4 val2 = tex1d.SampleGrad(0.5, 1, 2, 2);

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex3_load]] %float_0_5 Grad|ConstOffset|MinLod %float_1 %float_2 %int_2 %float_0_5
// CHECK: OpStore %val3 [[sampled_result_3]]
    float4 val3 = tex1d.SampleGrad(0.5, 1, 2, 2, 0.5);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[tex4_load]] %float_0_5 Grad|ConstOffset|MinLod %float_1 %float_2 %int_2 %float_0_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]
    uint status;
    float4 val4 = tex1d.SampleGrad(0.5, 1, 2, 2, 0.5, status);

    return 1.0;
}
