// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]
// CHECK: [[type_2d_image_array_uint:%[a-zA-Z0-9_]+]] = OpTypeImage %uint 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array_uint:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array_uint]]

vk::SampledTexture2DArray<float4> tex2dArrayf4;
vk::SampledTexture2DArray<uint> tex2dArrayuint;

float4 main() : SV_Target {

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArrayf4
// CHECK:      [[val1:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex1_load]] [[v2fc]] %int_0 None
    float4 val1 = tex2dArrayf4.Gather(float3(0.5, 0.25, 0.1));

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array_uint]] %tex2dArrayuint
// CHECK:      [[val2:%[a-zA-Z0-9_]+]] = OpImageGather %v4uint [[tex2_load]] [[v2fc]] %int_0 ConstOffset [[v2ic]]
    uint4 val2 = tex2dArrayuint.Gather(float3(0.5, 0.25, 0.1), int2(2, 3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2dArrayf4
// CHECK:      [[val3:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[tex3_load]] [[v2fc]] %int_0 ConstOffset [[v2ic]]
// CHECK:  [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[val3]] 0
// CHECK:                                OpStore %status [[status_0]]
    uint status;
    float4 val3 = tex2dArrayf4.Gather(float3(0.5, 0.25, 0.1), int2(2, 3), status);

    return 1.0;
}
