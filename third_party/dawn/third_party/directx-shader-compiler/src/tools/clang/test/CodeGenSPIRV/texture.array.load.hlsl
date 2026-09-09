// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

Texture1DArray <float4> t1 : register(t1);
Texture2DArray <float4> t2 : register(t2);
// .Load() does not support TextureCubeArray.

// CHECK: OpCapability SparseResidency

// CHECK: [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_2 %int_3
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float

float4 main(int4 location: A) : SV_Target {

// CHECK:      [[coord:%[0-9]+]] = OpVectorShuffle %v2int [[v3ic]] [[v3ic]] 0 1
// CHECK-NEXT:   [[lod:%[0-9]+]] = OpCompositeExtract %int [[v3ic]] 2
// CHECK-NEXT:    [[t1:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[t1]] [[coord]] Lod|ConstOffset [[lod]] %int_10
    float4 val1 = t1.Load(int3(1, 2, 3), 10);

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v4int %location
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v3int [[loc]] [[loc]] 0 1 2
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 3
// CHECK-NEXT:    [[t2:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[t2]] [[coord_0]] Lod [[lod_0]]
    float4 val2 = t2.Load(location);

    uint status;
// CHECK:            [[coord_1:%[0-9]+]] = OpVectorShuffle %v2int [[v3ic]] [[v3ic]] 0 1
// CHECK-NEXT:         [[lod_1:%[0-9]+]] = OpCompositeExtract %int [[v3ic]] 2
// CHECK-NEXT:          [[t1_0:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[t1_0]] [[coord_1]] Lod|ConstOffset [[lod_1]] %int_10
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:      [[result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                        OpStore %val3 [[result]]
    float4 val3 = t1.Load(int3(1, 2, 3), 10, status);

// CHECK:              [[loc_0:%[0-9]+]] = OpLoad %v4int %location
// CHECK-NEXT:       [[coord_2:%[0-9]+]] = OpVectorShuffle %v3int [[loc_0]] [[loc_0]] 0 1 2
// CHECK-NEXT:         [[lod_2:%[0-9]+]] = OpCompositeExtract %int [[loc_0]] 3
// CHECK-NEXT:          [[t2_0:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:[[structResult_0:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[t2_0]] [[coord_2]] Lod|ConstOffset [[lod_2]] [[v2ic]]
// CHECK-NEXT:      [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                        OpStore %status [[status_0]]
// CHECK-NEXT:      [[result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:                        OpStore %val4 [[result_0]]
    float4 val4 = t2.Load(location, int2(1, 2), status);

    return 1.0;
}
