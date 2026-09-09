// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

Texture1D       <float4> t1 : register(t1);
Texture2D       <float4> t2 : register(t2);
Texture3D       <float4> t3 : register(t3);
// .Load() does not support TextureCube.

Texture1D        <float> t4 : register(t4);
Texture2D        <int2>  t5 : register(t5);
Texture3D        <uint3> t6 : register(t6);

Texture2DMS     <float>  t7 : register(t7);
Texture2DMSArray<float3> t8 : register(t8);

Texture1D       <bool>   t9 : register(t9);
Texture2D       <bool>  t10 : register(t10);
Texture2D       <bool3> t11 : register(t11);

// CHECK: OpCapability SparseResidency

// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK: [[v4ic:%[0-9]+]] = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
// CHECK: [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_3 %int_3 %int_3
// CHECK: [[v3uint000:%[0-9]+]] = OpConstantComposite %v3uint %uint_0 %uint_0 %uint_0

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float
// CHECK: %SparseResidencyStruct_0 = OpTypeStruct %uint %v4int
// CHECK: %SparseResidencyStruct_1 = OpTypeStruct %uint %v4uint

float4 main(int3 location: A, int offset: B) : SV_Target {
    uint status;

// CHECK:      [[coord:%[0-9]+]] = OpCompositeExtract %int [[v2ic]] 0
// CHECK-NEXT:   [[lod:%[0-9]+]] = OpCompositeExtract %int [[v2ic]] 1
// CHECK-NEXT:    [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[t1]] [[coord]] Lod|ConstOffset [[lod]] %int_1
    float4 val1 = t1.Load(int2(1, 2), 1);

// CHECK:        [[loc:%[0-9]+]] = OpLoad %v3int %location
// CHECK-NEXT: [[coord_0:%[0-9]+]] = OpVectorShuffle %v2int [[loc]] [[loc]] 0 1
// CHECK-NEXT:   [[lod_0:%[0-9]+]] = OpCompositeExtract %int [[loc]] 2
// CHECK-NEXT:    [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[t2]] [[coord_0]] Lod|ConstOffset [[lod_0]] [[v2ic]]
    float4 val2 = t2.Load(location, int2(1, 2));

// CHECK:      [[coord_1:%[0-9]+]] = OpVectorShuffle %v3int [[v4ic]] [[v4ic]] 0 1 2
// CHECK-NEXT:   [[lod_1:%[0-9]+]] = OpCompositeExtract %int [[v4ic]] 3
// CHECK-NEXT:    [[t3:%[0-9]+]] = OpLoad %type_3d_image %t3
// CHECK-NEXT:       {{%[0-9]+}} = OpImageFetch %v4float [[t3]] [[coord_1]] Lod|ConstOffset [[lod_1]] [[v3ic]]
    float4 val3 = t3.Load(int4(1, 2, 3, 4), 3);

// CHECK:      [[f4:%[0-9]+]] = OpImageFetch %v4float {{%[0-9]+}} {{%[0-9]+}} Lod|ConstOffset {{%[0-9]+}} %int_1
// CHECK-NEXT:    {{%[0-9]+}} = OpCompositeExtract %float [[f4]] 0
    float val4 = t4.Load(int2(1,2), 1);

// CHECK:      [[f5:%[0-9]+]] = OpImageFetch %v4int {{%[0-9]+}} {{%[0-9]+}} Lod|ConstOffset {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:    {{%[0-9]+}} = OpVectorShuffle %v2int [[f5]] [[f5]] 0 1
    int2  val5 = t5.Load(location, int2(1,2));

// CHECK:      [[f6:%[0-9]+]] = OpImageFetch %v4uint {{%[0-9]+}} {{%[0-9]+}} Lod|ConstOffset {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:    {{%[0-9]+}} = OpVectorShuffle %v3uint [[f6]] [[f6]] 0 1 2
    uint3 val6 = t6.Load(int4(1, 2, 3, 4), 3);

    float val7;
    float3 val8;
    bool val9;
    bool3 val10;
    int sampleIndex = 7;
    int2 pos2 = int2(2, 3);
    int3 pos3 = int3(2, 3, 4);
    int2 offset2 = int2(1, 2);

// CHECK:     [[pos0:%[0-9]+]] = OpLoad %v2int %pos2
// CHECK-NEXT: [[si0:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT: [[t70:%[0-9]+]] = OpLoad %type_2d_image_1 %t7
// CHECK-NEXT: [[f70:%[0-9]+]] = OpImageFetch %v4float [[t70]] [[pos0]] Sample [[si0]]
// CHECK-NEXT:     {{%[0-9]+}} = OpCompositeExtract %float [[f70]] 0
    val7 = t7.Load(pos2, sampleIndex);

// CHECK:        [[pos1:%[0-9]+]] = OpLoad %v2int %pos2
// CHECK-NEXT:    [[si1:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT:    [[t71:%[0-9]+]] = OpLoad %type_2d_image_1 %t7
// CHECK-NEXT:    [[f71:%[0-9]+]] = OpImageFetch %v4float [[t71]] [[pos1]] ConstOffset|Sample [[v2ic]] [[si1]]
// CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[f71]] 0
    val7 = t7.Load(pos2, sampleIndex, int2(1, 2));

// CHECK:     [[pos2:%[0-9]+]] = OpLoad %v3int %pos3
// CHECK-NEXT: [[si2:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT: [[t80:%[0-9]+]] = OpLoad %type_2d_image_array %t8
// CHECK-NEXT: [[f80:%[0-9]+]] = OpImageFetch %v4float [[t80]] [[pos2]] Sample [[si2]]
// CHECK-NEXT:     {{%[0-9]+}} = OpVectorShuffle %v3float [[f80]] [[f80]] 0 1 2
    val8 = t8.Load(pos3, sampleIndex);

// CHECK:     [[pos3:%[0-9]+]] = OpLoad %v3int %pos3
// CHECK-NEXT: [[si3:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT: [[t81:%[0-9]+]] = OpLoad %type_2d_image_array %t8
// CHECK-NEXT: [[f81:%[0-9]+]] = OpImageFetch %v4float [[t81]] [[pos3]] ConstOffset|Sample [[v2ic]] [[si3]]
// CHECK-NEXT:     {{%[0-9]+}} = OpVectorShuffle %v3float [[f81]] [[f81]] 0 1 2
    val8 = t8.Load(pos3, sampleIndex, int2(1,2));

// CHECK:       [[v0:%[0-9]+]] = OpLoad %v2int %pos2
// CHECK-NEXT:  [[v1:%[0-9]+]] = OpCompositeExtract %int [[v0]] 0
// CHECK-NEXT:  [[v2:%[0-9]+]] = OpCompositeExtract %int [[v0]] 1
// CHECK-NEXT:  [[v3:%[0-9]+]] = OpLoad %type_1d_image_0 %t9
// CHECK-NEXT:  [[v4:%[0-9]+]] = OpImageFetch %v4uint [[v3]] [[v1]] Lod [[v2]]
// CHECK-NEXT:  [[v5:%[0-9]+]] = OpCompositeExtract %uint [[v4]] 0
// CHECK-NEXT:  [[v6:%[0-9]+]] = OpINotEqual %bool [[v5]] %uint_0
// CHECK-NEXT:                OpStore %val9 [[v6]]
    val9 = t9.Load(pos2);

// CHECK-NEXT:  [[v10:%[0-9]+]] = OpLoad %v3int %pos3
// CHECK-NEXT:  [[v11:%[0-9]+]] = OpVectorShuffle %v2int [[v10]] [[v10]] 0 1
// CHECK-NEXT:  [[v12:%[0-9]+]] = OpCompositeExtract %int [[v10]] 2
// CHECK-NEXT:  [[v13:%[0-9]+]] = OpLoad %type_2d_image_2 %t10
// CHECK-NEXT:  [[v14:%[0-9]+]] = OpImageFetch %v4uint [[v13]] [[v11]] Lod [[v12]]
// CHECK-NEXT:  [[v15:%[0-9]+]] = OpCompositeExtract %uint [[v14]] 0
// CHECK-NEXT:  [[v16:%[0-9]+]] = OpINotEqual %bool [[v15]] %uint_0
// CHECK-NEXT:                 OpStore %val9 [[v16]]
    val9 = t10.Load(pos3);

// CHECK-NEXT:  [[v20:%[0-9]+]] = OpLoad %v3int %pos3
// CHECK-NEXT:  [[v21:%[0-9]+]] = OpVectorShuffle %v2int [[v20]] [[v20]] 0 1
// CHECK-NEXT:  [[v22:%[0-9]+]] = OpCompositeExtract %int [[v20]] 2
// CHECK-NEXT:  [[v23:%[0-9]+]] = OpLoad %type_2d_image_2 %t11
// CHECK-NEXT:  [[v24:%[0-9]+]] = OpImageFetch %v4uint [[v23]] [[v21]] Lod [[v22]]
// CHECK-NEXT:  [[v25:%[0-9]+]] = OpVectorShuffle %v3uint [[v24]] [[v24]] 0 1 2
// CHECK-NEXT:  [[v26:%[0-9]+]] = OpINotEqual %v3bool [[v25]] [[v3uint000]]
// CHECK-NEXT:                 OpStore %val10 [[v26]]
    val10 = t11.Load(pos3);

/////////////////////////////////
/// Using the Status argument ///
/////////////////////////////////

// CHECK:            [[coord_2:%[0-9]+]] = OpCompositeExtract %int [[v2ic]] 0
// CHECK-NEXT:         [[lod_2:%[0-9]+]] = OpCompositeExtract %int [[v2ic]] 1
// CHECK-NEXT:          [[t4:%[0-9]+]] = OpLoad %type_1d_image %t4
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[t4]] [[coord_2]] Lod|ConstOffset [[lod_2]] %int_1
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:    [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:      [[result:%[0-9]+]] = OpCompositeExtract %float [[v4result]] 0
// CHECK-NEXT:                        OpStore %val14 [[result]]
    float  val14 = t4.Load(int2(1,2), 1, status);

// CHECK:              [[loc_0:%[0-9]+]] = OpLoad %v3int %location
// CHECK-NEXT:       [[coord_3:%[0-9]+]] = OpVectorShuffle %v2int [[loc_0]] [[loc_0]] 0 1
// CHECK-NEXT:         [[lod_3:%[0-9]+]] = OpCompositeExtract %int [[loc_0]] 2
// CHECK-NEXT:          [[t5:%[0-9]+]] = OpLoad %type_2d_image_0 %t5
// CHECK-NEXT:[[structResult_0:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct_0 [[t5]] [[coord_3]] Lod|ConstOffset [[lod_3]] [[v2ic]]
// CHECK-NEXT:      [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                        OpStore %status [[status_0]]
// CHECK-NEXT:    [[v4result_0:%[0-9]+]] = OpCompositeExtract %v4int [[structResult_0]] 1
// CHECK-NEXT:      [[result_0:%[0-9]+]] = OpVectorShuffle %v2int [[v4result_0]] [[v4result_0]] 0 1
// CHECK-NEXT:                        OpStore %val15 [[result_0]]
    int2   val15 = t5.Load(location, int2(1,2), status);

// CHECK:            [[coord_4:%[0-9]+]] = OpVectorShuffle %v3int [[v4ic]] [[v4ic]] 0 1 2
// CHECK-NEXT:         [[lod_4:%[0-9]+]] = OpCompositeExtract %int [[v4ic]] 3
// CHECK-NEXT:          [[t6:%[0-9]+]] = OpLoad %type_3d_image_0 %t6
// CHECK-NEXT:[[structResult_1:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct_1 [[t6]] [[coord_4]] Lod|ConstOffset [[lod_4]] [[v3ic]]
// CHECK-NEXT:      [[status_1:%[0-9]+]] = OpCompositeExtract %uint [[structResult_1]] 0
// CHECK-NEXT:                        OpStore %status [[status_1]]
// CHECK-NEXT:    [[v4result_1:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult_1]] 1
// CHECK-NEXT:      [[result_1:%[0-9]+]] = OpVectorShuffle %v3uint [[v4result_1]] [[v4result_1]] 0 1 2
// CHECK-NEXT:                        OpStore %val16 [[result_1]]
    uint3  val16 = t6.Load(int4(1, 2, 3, 4), 3, status);

// CHECK:             [[pos1_0:%[0-9]+]] = OpLoad %v2int %pos2
// CHECK-NEXT:         [[si1_0:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT:         [[t71_0:%[0-9]+]] = OpLoad %type_2d_image_1 %t7
// CHECK-NEXT:[[structResult_2:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[t71_0]] [[pos1_0]] ConstOffset|Sample [[v2ic]] [[si1_0]]
// CHECK-NEXT:      [[status_2:%[0-9]+]] = OpCompositeExtract %uint [[structResult_2]] 0
// CHECK-NEXT:                        OpStore %status [[status_2]]
// CHECK-NEXT:    [[v4result_2:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_2]] 1
// CHECK-NEXT:      [[result_2:%[0-9]+]] = OpCompositeExtract %float [[v4result_2]] 0
// CHECK-NEXT:                        OpStore %val17 [[result_2]]
    float  val17 = t7.Load(pos2, sampleIndex, int2(1,2), status);

// CHECK:             [[pos3_0:%[0-9]+]] = OpLoad %v3int %pos3
// CHECK-NEXT:         [[si3_0:%[0-9]+]] = OpLoad %int %sampleIndex
// CHECK-NEXT:         [[t81_0:%[0-9]+]] = OpLoad %type_2d_image_array %t8
// CHECK-NEXT:[[structResult_3:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[t81_0]] [[pos3_0]] ConstOffset|Sample [[v2ic]] [[si3_0]]
// CHECK-NEXT:      [[status_3:%[0-9]+]] = OpCompositeExtract %uint [[structResult_3]] 0
// CHECK-NEXT:                        OpStore %status [[status_3]]
// CHECK-NEXT:    [[v4result_3:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_3]] 1
// CHECK-NEXT:      [[result_3:%[0-9]+]] = OpVectorShuffle %v3float [[v4result_3]] [[v4result_3]] 0 1 2
// CHECK-NEXT:                        OpStore %val18 [[result_3]]
    float3 val18 = t8.Load(pos3, sampleIndex, int2(1,2), status);

    return 1.0;
}
