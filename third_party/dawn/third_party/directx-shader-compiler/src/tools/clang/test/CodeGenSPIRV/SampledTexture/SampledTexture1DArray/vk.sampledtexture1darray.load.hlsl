// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

float4 main(int3 location3: A, int4 location4: B) : SV_Target {
// CHECK: %location3 = OpFunctionParameter %_ptr_Function_v3int
// CHECK: %location4 = OpFunctionParameter %_ptr_Function_v4int
// CHECK: [[location3:%[a-zA-Z0-9_]+]] = OpLoad %v3int %location3
// CHECK: [[coords:%[a-zA-Z0-9_]+]] = OpVectorShuffle %v2int [[location3]] [[location3]] 0 1
// CHECK: [[mip_level:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3]] 2
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image_array [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[coords]] Lod [[mip_level]]
// CHECK: OpStore %val1 [[fetch_result]]
// CHECK: [[location3_2:%[a-zA-Z0-9_]+]] = OpLoad %v3int %location3
// CHECK: [[coords_2:%[a-zA-Z0-9_]+]] = OpVectorShuffle %v2int [[location3_2]] [[location3_2]] 0 1
// CHECK: [[mip_level_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_2]] 2
// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[tex_image_2:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image_array [[tex2_load]]
// CHECK: [[fetch_result_2:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image_2]] [[coords_2]] Lod|ConstOffset [[mip_level_2]] %int_1
// CHECK: OpStore %val2 [[fetch_result_2]]
// CHECK: [[location3_3:%[a-zA-Z0-9_]+]] = OpLoad %v3int %location3
// CHECK: [[coords_3:%[a-zA-Z0-9_]+]] = OpVectorShuffle %v2int [[location3_3]] [[location3_3]] 0 1
// CHECK: [[mip_level_3:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_3]] 2
// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[tex_image_3:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image_array [[tex3_load]]
// CHECK: [[sparse_fetch_result:%[a-zA-Z0-9_]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_image_3]] [[coords_3]] Lod|ConstOffset [[mip_level_3]] %int_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sparse_fetch_result]] 0
// CHECK: OpStore %status [[status_0]]

    uint status;

// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sparse_fetch_result]] 1
// CHECK: OpStore %val3 [[sampled_texel]]

    float4 val1 = tex1dArray.Load(location3);

    float4 val2 = tex1dArray.Load(location3, int2(1, 2));

/////////////////////////////////
/// Using the Status argument ///
/////////////////////////////////

    float4  val3 = tex1dArray.Load(location3, int2(1, 2), status);

    return 1.0;
}
