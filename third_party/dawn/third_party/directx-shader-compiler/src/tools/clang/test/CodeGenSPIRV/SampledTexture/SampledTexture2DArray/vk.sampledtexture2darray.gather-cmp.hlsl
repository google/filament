// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

float4 main() : SV_Target {
    uint status;
    float4 val = 0;

// CHECK: [[tex1_load_1:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[val_1:%[a-zA-Z0-9_]+]] = OpImageDrefGather %v4float [[tex1_load_1]] [[v2fc]] %float_0_5
// CHECK: OpStore %val [[val_1]]
    val = tex2darray.GatherCmp(float3(0.5, 0.25, 0.1), 0.5);

// CHECK: [[tex1_load_2:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[val_2:%[a-zA-Z0-9_]+]] = OpImageDrefGather %v4float [[tex1_load_2]] [[v2fc]] %float_0_5 ConstOffset [[v2ic]]
// CHECK: OpStore %val [[val_2]]
    val = tex2darray.GatherCmp(float3(0.5, 0.25, 0.1), 0.5, int2(1, 2));

// CHECK: [[tex1_load_3:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK: [[val_struct:%[a-zA-Z0-9_]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[tex1_load_3]] [[v2fc]] %float_0_5 ConstOffset [[v2ic]]
// CHECK: [[status_1:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[val_struct]] 0
// CHECK: OpStore %status [[status_1]]
// CHECK: [[res_1:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[val_struct]] 1
// CHECK: OpStore %val [[res_1]]
    val = tex2darray.GatherCmp(float3(0.5, 0.25, 0.1), 0.5, int2(1, 2), status);

    return val;
}
