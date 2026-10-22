// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0
// CHECK: [[v3f_1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v3f_2:%[0-9]+]] = OpConstantComposite %v3float %float_2 %float_2 %float_2
// CHECK: [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_2 %int_3 %int_1

// CHECK: [[type_3d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image]]

vk::SampledTexture3D<float4> tex3d;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex1_load]] [[v3fc]] Grad [[v3f_1]] [[v3f_2]]
    float4 val1 = tex3d.SampleGrad(float3(0.5, 0.25, 0), float3(1, 1, 1), float3(2, 2, 2));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex2_load]] [[v3fc]] Grad|ConstOffset [[v3f_1]] [[v3f_2]] [[v3ic]]
    float4 val2 = tex3d.SampleGrad(float3(0.5, 0.25, 0), float3(1, 1, 1), float3(2, 2, 2), int3(2, 3, 1));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleExplicitLod %v4float [[tex3_load]] [[v3fc]] Grad|ConstOffset|MinLod [[v3f_1]] [[v3f_2]] [[v3ic]] %float_0_5
    float4 val3 = tex3d.SampleGrad(float3(0.5, 0.25, 0), float3(1, 1, 1), float3(2, 2, 2), int3(2, 3, 1), 0.5);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[tex4_load]] [[v3fc]] Grad|ConstOffset|MinLod [[v3f_1]] [[v3f_2]] [[v3ic]] %float_0_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK: OpStore %status [[status_0]]
// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sampled_result4]] 1
// CHECK: OpStore %val4 [[sampled_texel]]
    uint status;
    float4 val4 = tex3d.SampleGrad(float3(0.5, 0.25, 0), float3(1, 1, 1), float3(2, 2, 2), int3(2, 3, 1), 0.5, status);

    return 1.0;
}
