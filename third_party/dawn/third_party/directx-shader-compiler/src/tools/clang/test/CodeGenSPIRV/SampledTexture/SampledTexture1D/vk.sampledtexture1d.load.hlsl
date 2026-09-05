// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

float4 main(int2 location3: A) : SV_Target {
// CHECK: %location3 = OpFunctionParameter %_ptr_Function_v2int

// CHECK: [[location3:%[a-zA-Z0-9_]+]] = OpLoad %v2int %location3
// CHECK: [[coord:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3]] 0
// CHECK: [[mip_level:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3]] 1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[coord]] Lod [[mip_level]]
// CHECK: OpStore %val1 [[fetch_result]]
    float4 val1 = tex1d.Load(location3);

// CHECK: [[location3_2:%[a-zA-Z0-9_]+]] = OpLoad %v2int %location3
// CHECK: [[coord_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_2]] 0
// CHECK: [[mip_level_2:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_2]] 1
// CHECK: [[tex2_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[tex_image_2:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image [[tex2_load]]
// CHECK: [[fetch_result_2:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image_2]] [[coord_2]] Lod|ConstOffset [[mip_level_2]] %int_1
// CHECK: OpStore %val2 [[fetch_result_2]]
    float4 val2 = tex1d.Load(location3, 1);

/////////////////////////////////
/// Using the Status argument ///
/////////////////////////////////

// CHECK: [[location3_3:%[a-zA-Z0-9_]+]] = OpLoad %v2int %location3
// CHECK: [[coord_3:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_3]] 0
// CHECK: [[mip_level_3:%[a-zA-Z0-9_]+]] = OpCompositeExtract %int [[location3_3]] 1
// CHECK: [[tex3_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[tex_image_3:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image [[tex3_load]]
// CHECK: [[sparse_fetch_result:%[a-zA-Z0-9_]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_image_3]] [[coord_3]] Lod|ConstOffset [[mip_level_3]] %int_1
// CHECK: [[status_0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[sparse_fetch_result]] 0
// CHECK: OpStore %status [[status_0]]

// CHECK: [[sampled_texel:%[a-zA-Z0-9_]+]] = OpCompositeExtract %v4float [[sparse_fetch_result]] 1
// CHECK: OpStore %val3 [[sampled_texel]]
    uint status;
    float4  val3 = tex1d.Load(location3, 1, status);

    return 1.0;
}
