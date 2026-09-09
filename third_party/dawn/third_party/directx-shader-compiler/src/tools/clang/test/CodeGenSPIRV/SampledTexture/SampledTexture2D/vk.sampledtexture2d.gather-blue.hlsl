// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_0_5 %float_0_25
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_3

// CHECK: [[type_2d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 0 1 Unknown
// CHECK: [[type_2d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image]]

vk::SampledTexture2D<float4> tex2d;

float4 main() : SV_Target {
    uint status;
    float4 val = 0;

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[val_blue:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex1_load]] [[v2fc]] %int_2
// CHECK: OpStore %val [[val_blue]]
    val = tex2d.GatherBlue(float2(0.5, 0.25));

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[val_blue_o:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex1_load]] [[v2fc]] %int_2 ConstOffset [[v2ic]]
// CHECK: OpStore %val [[val_blue_o]]
    val = tex2d.GatherBlue(float2(0.5, 0.25), int2(2, 3));

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[val_blue_o4:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex1_load]] [[v2fc]] %int_2 ConstOffsets [[const_offsets:%[a-zA-Z0-9_]+]]
// CHECK: OpStore %val [[val_blue_o4]]
    val = tex2d.GatherBlue(float2(0.5, 0.25), int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[val_blue_s:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[tex1_load]] [[v2fc]] %int_2 ConstOffset [[v2ic]]
// CHECK: [[status_blue_s:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[val_blue_s]] 0
// CHECK: OpStore %status [[status_blue_s]]
// CHECK: [[res_blue_s:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[val_blue_s]] 1
// CHECK: OpStore %val [[res_blue_s]]
    val = tex2d.GatherBlue(float2(0.5, 0.25), int2(2, 3), status);

// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK: [[val_blue_o4_s:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[tex1_load]] [[v2fc]] %int_2 ConstOffsets [[const_offsets]]
// CHECK: [[status_blue_o4_s:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[val_blue_o4_s]] 0
// CHECK: OpStore %status [[status_blue_o4_s]]
// CHECK: [[res_blue_o4_s:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[val_blue_o4_s]] 1
// CHECK: OpStore %val [[res_blue_o4_s]]
    val = tex2d.GatherBlue(float2(0.5, 0.25), int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8), status);

    return val;
}
