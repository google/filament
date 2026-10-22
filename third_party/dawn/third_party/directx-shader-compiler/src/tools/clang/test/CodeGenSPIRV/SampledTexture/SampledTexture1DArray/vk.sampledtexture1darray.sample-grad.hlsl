// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sample_ddx:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sample_ddy:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_2:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex1_load]] [[sample_coord:%[a-zA-Z0-9_]+]] Grad [[sample_grad:%[a-zA-Z0-9_]+]] [[sample_grad_2:%[a-zA-Z0-9_]+]]
// CHECK: OpStore %val1 [[sampled_result]]

    float4 val1 = tex1dArray.SampleGrad(float2(0.5, 0.25), float2(1, 1), float2(2, 2));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sample_ddx_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_3:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sample_ddy_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_4:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex2_load]] [[sample_coord_2:%[a-zA-Z0-9_]+]] Grad|ConstOffset [[sample_grad_3:%[a-zA-Z0-9_]+]] [[sample_grad_4:%[a-zA-Z0-9_]+]] %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]

    float4 val2 = tex1dArray.SampleGrad(float2(0.5, 0.25), float2(1, 1), float2(2, 2), int2(2,3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sample_ddx_3:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_5:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sample_ddy_3:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_6:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex3_load]] [[sample_coord_3:%[a-zA-Z0-9_]+]] Grad|ConstOffset|MinLod [[sample_grad_5:%[a-zA-Z0-9_]+]] [[sample_grad_6:%[a-zA-Z0-9_]+]] %int_2 %float_0_5
// CHECK: OpStore %val3 [[sampled_result_3]]

    float4 val3 = tex1dArray.SampleGrad(float2(0.5, 0.25), float2(1, 1), float2(2, 2), int2(2,3), 0.5);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sample_ddx_4:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_7:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sample_ddy_4:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[grad_vec_8:%[a-zA-Z0-9_]+]] 0
// CHECK: [[sampled_result_4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[tex4_load]] [[sample_coord_4:%[a-zA-Z0-9_]+]] Grad|ConstOffset|MinLod [[sample_grad_7:%[a-zA-Z0-9_]+]] [[sample_grad_8:%[a-zA-Z0-9_]+]] %int_2 %float_0_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_4]] 0
// CHECK: OpStore %status [[status_0]]

    uint status;
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result_4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]

    float4 val4 = tex1dArray.SampleGrad(float2(0.5, 0.25), float2(1, 1), float2(2, 2), int2(2,3), 0.5, status);

    return 1.0;
}
