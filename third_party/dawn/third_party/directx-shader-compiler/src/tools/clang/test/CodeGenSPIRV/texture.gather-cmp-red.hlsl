// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState gSampler : register(s5);

Texture2D<float4> t2f4  : register(t1);
Texture2D<uint>   t2u1  : register(t2);
TextureCube<int>  tCube : register(t3);
// .GatherCmpRed() does not support Texture1D and Texture3D.

// CHECK: OpCapability SparseResidency

// CHECK:      [[c12:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK:      [[c34:%[0-9]+]] = OpConstantComposite %v2int %int_3 %int_4
// CHECK:      [[c56:%[0-9]+]] = OpConstantComposite %v2int %int_5 %int_6
// CHECK:      [[c78:%[0-9]+]] = OpConstantComposite %v2int %int_7 %int_8
// CHECK:    [[c1to8:%[0-9]+]] = OpConstantComposite %_arr_v2int_uint_4 [[c12]] [[c34]] [[c56]] [[c78]]
// CHECK: [[cv3f_1_5:%[0-9]+]] = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5

float4 main(float2 location: A, float comparator: B) : SV_Target {
// CHECK:            [[t2f4:%[0-9]+]] = OpLoad %type_2d_image %t2f4
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2f4]] [[gSampler]]
// CHECK-NEXT:        [[res:%[0-9]+]] = OpImageDrefGather %v4float [[sampledImg]] [[loc]] [[comparator]] ConstOffset [[c12]]
// CHECK-NEXT:                       OpStore %a [[res]]
    float4 a = t2f4.GatherCmpRed(gSampler, location, comparator, int2(1, 2));
// CHECK:            [[t2f4_0:%[0-9]+]] = OpLoad %type_2d_image %t2f4
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_0:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator_0:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2f4_0]] [[gSampler_0]]
// CHECK-NEXT:        [[res_0:%[0-9]+]] = OpImageDrefGather %v4float [[sampledImg_0]] [[loc_0]] [[comparator_0]] ConstOffsets [[c1to8]]
// CHECK-NEXT:                       OpStore %b [[res_0]]
    float4 b = t2f4.GatherCmpRed(gSampler, location, comparator, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));

// CHECK:            [[t2u1:%[0-9]+]] = OpLoad %type_2d_image_0 %t2u1
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_1:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator_1:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u1]] [[gSampler_1]]
// CHECK-NEXT:        [[res_1:%[0-9]+]] = OpImageDrefGather %v4uint [[sampledImg_1]] [[loc_1]] [[comparator_1]] ConstOffset [[c12]]
// CHECK-NEXT:                       OpStore %c [[res_1]]
    uint4 c = t2u1.GatherCmpRed(gSampler, location, comparator, int2(1, 2));
// CHECK:            [[t2u1_0:%[0-9]+]] = OpLoad %type_2d_image_0 %t2u1
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_2:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator_2:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u1_0]] [[gSampler_2]]
// CHECK-NEXT:        [[res_2:%[0-9]+]] = OpImageDrefGather %v4uint [[sampledImg_2]] [[loc_2]] [[comparator_2]] ConstOffsets [[c1to8]]
// CHECK-NEXT:                       OpStore %d [[res_2]]
    uint4 d = t2u1.GatherCmpRed(gSampler, location, comparator, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));

    uint status;
// CHECK:             [[t2u1_1:%[0-9]+]] = OpLoad %type_2d_image_0 %t2u1
// CHECK-NEXT:    [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:         [[loc_3:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT:  [[comparator_3:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:  [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u1_1]] [[gSampler_3]]
// CHECK-NEXT:[[structResult:%[0-9]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[sampledImg_3]] [[loc_3]] [[comparator_3]] ConstOffset [[c12]]
// CHECK-NEXT:      [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                        OpStore %status [[status]]
// CHECK-NEXT:      [[result:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult]] 1
// CHECK-NEXT:                        OpStore %e [[result]]
    uint4 e = t2u1.GatherCmpRed(gSampler, location, comparator, int2(1, 2), status);

// CHECK:             [[t2u1_2:%[0-9]+]] = OpLoad %type_2d_image_0 %t2u1
// CHECK-NEXT:    [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:         [[loc_4:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT:  [[comparator_4:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:  [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u1_2]] [[gSampler_4]]
// CHECK-NEXT:[[structResult_0:%[0-9]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[sampledImg_4]] [[loc_4]] [[comparator_4]] ConstOffsets [[c1to8]]
// CHECK-NEXT:      [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                        OpStore %status [[status_0]]
// CHECK-NEXT:      [[result_0:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult_0]] 1
// CHECK-NEXT:                        OpStore %f [[result_0]]
    uint4 f = t2u1.GatherCmpRed(gSampler, location, comparator, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8), status);

// CHECK:            [[tCube:%[0-9]+]] = OpLoad %type_cube_image %tCube
// CHECK-NEXT:    [[gSampler_5:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:  [[sampledImg_5:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[tCube]] [[gSampler_5]]
// CHECK-NEXT:[[structResult_1:%[0-9]+]] = OpImageSparseDrefGather %SparseResidencyStruct_0 [[sampledImg_5]] [[cv3f_1_5]] %float_2_5 None
// CHECK-NEXT:      [[status_1:%[0-9]+]] = OpCompositeExtract %uint [[structResult_1]] 0
// CHECK-NEXT:                        OpStore %status [[status_1]]
// CHECK-NEXT:      [[result_1:%[0-9]+]] = OpCompositeExtract %v4int [[structResult_1]] 1
// CHECK-NEXT:                        OpStore %g [[result_1]]
    int4 g = tCube.GatherCmpRed(gSampler, /*location*/ float3(1.5, 1.5, 1.5), /*compare_value*/ 2.5, status);

    return 1.0;
}
