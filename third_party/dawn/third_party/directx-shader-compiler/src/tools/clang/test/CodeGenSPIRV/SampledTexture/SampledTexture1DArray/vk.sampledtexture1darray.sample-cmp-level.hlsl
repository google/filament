// RUN: %dxc -T ps_6_0 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

float4 main() : SV_Target {
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result:%[a-zA-Z0-9_]+]] = OpImageSampleDrefExplicitLod %float [[tex1_load]] [[sample_coord:%[a-zA-Z0-9_]+]] %float_2 Lod %float_1
// CHECK: OpStore %val1 [[sampled_result]]

    float val1 = tex1dArray.SampleCmpLevel(float2(0.5, 0.25), 2.0f, 1.0f);

// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result_2:%[a-zA-Z0-9_]+]] = OpImageSampleDrefExplicitLod %float [[tex2_load]] [[sample_coord_2:%[a-zA-Z0-9_]+]] %float_2 Lod|ConstOffset %float_1 %int_2
// CHECK: OpStore %val2 [[sampled_result_2]]

    float val2 = tex1dArray.SampleCmpLevel(float2(0.5, 0.25), 2.0f, 1.0f, int2(2,3));

// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[sampled_result_3:%[a-zA-Z0-9_]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[tex3_load]] [[sample_coord_3:%[a-zA-Z0-9_]+]] %float_2 Lod|ConstOffset %float_1 %int_2
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sampled_result_3]] 0
// CHECK: OpStore %status [[status_0]]

    uint status;
// CHECK: [[sampled_value:%[a-zA-Z0-9_]+]] = OpCompositeExtract %float [[sampled_result_3]] 1
// CHECK: OpStore %val3 [[sampled_value]]

    float val3 = tex1dArray.SampleCmpLevel(float2(0.5, 0.25), 2.0f, 1.0f, int2(2,3), status);

    return 1.0;
}
