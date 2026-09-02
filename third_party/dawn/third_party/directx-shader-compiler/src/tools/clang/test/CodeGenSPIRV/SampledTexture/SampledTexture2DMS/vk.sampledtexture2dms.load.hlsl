// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2

// CHECK: [[type_2d_image_ms:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms]]

vk::SampledTexture2DMS<float4> tex2dMS;

float4 main(int3 location3: A) : SV_Target {
    uint status;

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v3int %location3
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v2int [[loc_ms]] [[loc_ms]] 0 1
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location3 %int_2
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[tex_ms]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_ms_img]] [[coord_ms]] Sample [[sample_ms]]
    float4 val1 = tex2dMS.Load(location3.xy, location3.z);

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v3int %location3
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v2int [[loc_ms]] [[loc_ms]] 0 1
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location3 %int_2
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[tex_ms]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_ms_img]] [[coord_ms]] ConstOffset|Sample [[v2ic]] [[sample_ms]]
    float4 val2 = tex2dMS.Load(location3.xy, location3.z, int2(1, 2));

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v3int %location3
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v2int [[loc_ms]] [[loc_ms]] 0 1
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location3 %int_2
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[tex_ms]]
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_ms_img]] [[coord_ms]] ConstOffset|Sample [[v2ic]] [[sample_ms]]
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:    [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                        OpStore %val3 [[v4result]]
    float4 val3 = tex2dMS.Load(location3.xy, location3.z, int2(1, 2), status);

    return 1.0;
}
