// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2

// CHECK: [[type_2d_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms_array]]

vk::SampledTexture2DMSArray<float4> tex2dMSArray;

float4 main(int4 location4: A) : SV_Target {
    uint status;

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v3int [[loc_ms]] [[loc_ms]] 0 1 2
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location4 %int_3
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[tex_ms]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_ms_img]] [[coord_ms]] Sample [[sample_ms]]
    float4 val1 = tex2dMSArray.Load(location4.xyz, location4.w);

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v3int [[loc_ms]] [[loc_ms]] 0 1 2
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location4 %int_3
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[tex_ms]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_ms_img]] [[coord_ms]] ConstOffset|Sample [[v2ic]] [[sample_ms]]
    float4 val2 = tex2dMSArray.Load(location4.xyz, location4.w, int2(1, 2));

// CHECK:        [[loc_ms:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT:[[coord_ms:%[0-9]+]] = OpVectorShuffle %v3int [[loc_ms]] [[loc_ms]] 0 1 2
// CHECK-NEXT:[[sample_ms_ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_int %location4 %int_3
// CHECK-NEXT:    [[sample_ms:%[0-9]+]] = OpLoad %int [[sample_ms_ptr]]
// CHECK-NEXT:    [[tex_ms:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT:[[tex_ms_img:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[tex_ms]]
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_ms_img]] [[coord_ms]] ConstOffset|Sample [[v2ic]] [[sample_ms]]
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:    [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                        OpStore %val3 [[v4result]]
    float4 val3 = tex2dMSArray.Load(location4.xyz, location4.w, int2(1, 2), status);

    return 1.0;
}
