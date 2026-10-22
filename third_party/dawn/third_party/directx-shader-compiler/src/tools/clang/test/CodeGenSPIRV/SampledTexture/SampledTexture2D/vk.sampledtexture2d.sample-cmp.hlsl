// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_0_5 %float_0_25
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 0 1 Unknown
// CHECK: [[type_2d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image]]

vk::SampledTexture2D<float4> tex2d;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[sampled_result1:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex1_load]] [[v2fc]] %float_2
    float val1 = tex2d.SampleCmp(float2(0.5, 0.25), 2.0f);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[sampled_result2:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex2_load]] [[v2fc]] %float_2 ConstOffset [[v2ic]]
    float val2 = tex2d.SampleCmp(float2(0.5, 0.25), 2.0f, int2(2,3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[sampled_result3:%[a-zA-Z0-9_]+]] = OpImageSampleDrefImplicitLod %float [[tex3_load]] [[v2fc]] %float_2 ConstOffset|MinLod [[v2ic]] %float_0_5
    float val3 = tex2d.SampleCmp(float2(0.5, 0.25), 2.0f, int2(2,3), 0.5);

// CHECK: [[tex4_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[sampled_result4:%[a-zA-Z0-9_]+]] = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[tex4_load]] [[v2fc]] %float_2 ConstOffset|MinLod [[v2ic]] %float_0_5
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result4]] 0
// CHECK:                        OpStore %status [[status_0]]
    uint status;
    float val4 = tex2d.SampleCmp(float2(0.5, 0.25), 2.0f, int2(2,3), 0.5, status);

        return 1.0;
}
