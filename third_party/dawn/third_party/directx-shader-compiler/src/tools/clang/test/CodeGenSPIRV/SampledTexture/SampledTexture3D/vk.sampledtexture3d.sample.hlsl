// RUN: %dxc -T ps_6_7 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0
// CHECK: [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_2 %int_3 %int_1

// CHECK: [[type_3d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image]]
// CHECK: [[type_3d_image_uint:%[a-zA-Z0-9_]+]] = OpTypeImage %uint 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image_uint:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image_uint]]

vk::SampledTexture3D<float4> tex3df4;
vk::SampledTexture3D<uint> tex3duint;
vk::SampledTexture3D tex3d_default;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3df4
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1_load]] [[v3fc]] None
    float4 val1 = tex3df4.Sample(float3(0.5, 0.25, 0));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d_default
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex2_load]] [[v3fc]] ConstOffset [[v3ic]]
    float4 val2 = tex3d_default.Sample(float3(0.5, 0.25, 0), int3(2, 3, 1));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3df4
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex3_load]] [[v3fc]] ConstOffset|MinLod [[v3ic]] %float_1
    float4 val3 = tex3df4.Sample(float3(0.5, 0.25, 0), int3(2, 3, 1), 1.0f);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3df4
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex4_load]] [[v3fc]] ConstOffset|MinLod [[v3ic]] %float_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK:                        OpStore %status [[status_0]]
    uint status;
    float4 val4 = tex3df4.Sample(float3(0.5, 0.25, 0), int3(2, 3, 1), 1.0f, status);

// CHECK: [[tex5_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image_uint]] %tex3duint
// CHECK: [[sampled_result5:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4uint [[tex5_load]] [[v3fc]] None
// CHECK: [[val5:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result5]] 0
    uint val5 = tex3duint.Sample(float3(0.5, 0.25, 0));

    return 1.0;
}
