// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:  [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_2 %int_3

// CHECK: [[type_3d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image]]

vk::SampledTexture3D<float4> tex3d;

float4 main(int4 location4: A) : SV_Target {
    uint status;

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_3d_image]] [[tex]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_img]] [[coord_0]] Lod [[lod_0]]
    float4 val1 = tex3d.Load(location4);

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_3d_image]] [[tex]]
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[tex_img]] [[coord_0]] Lod|ConstOffset [[lod_0]] [[v3ic]]
    float4 val2 = tex3d.Load(location4, int3(1, 2, 3));

/////////////////////////////////
/// Using the Status argument ///
/////////////////////////////////

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location4
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[tex:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:    [[tex_img:%[0-9]+]] = OpImage [[type_3d_image]] [[tex]]
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[tex_img]] [[coord_0]] Lod|ConstOffset [[lod_0]] [[v3ic]]
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:    [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                        OpStore %val3 [[v4result]]
    float4  val3 = tex3d.Load(location4, int3(1, 2, 3), status);

    return 1.0;
}
