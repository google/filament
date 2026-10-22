// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

float4 main(int3 location3: A, int4 location4: B) : SV_Target {
    uint status;

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_2d_image_array]] [[tex]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_img]] [[coord_0]] Lod [[lod_0]]
    float4 val1 = tex2darray.Load(location4);

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_2d_image_array]] [[tex]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_img]] [[coord_0]] Lod|ConstOffset [[lod_0]] [[v2ic]]
    float4 val2 = tex2darray.Load(location4, int2(1, 2));

/////////////////////////////////
/// Using the Status argument ///
/////////////////////////////////

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_2d_image_array]] [[tex]]
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_img]] [[coord_0]] Lod|ConstOffset [[lod_0]] [[v2ic]]
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:    [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                        OpStore %val3 [[v4result]]
    float4  val3 = tex2darray.Load(location4, int2(1, 2), status);

    return 1.0;
}
