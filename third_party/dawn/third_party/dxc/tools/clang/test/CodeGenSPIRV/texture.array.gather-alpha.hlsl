// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState gSampler : register(s5);

Texture2DArray<float4>  t2f4       : register(t1);
Texture2DArray<uint4>   t2u4       : register(t2);
TextureCubeArray<uint4> tCubeArray : register(t3);
// .GatherAlpha() does not support Texture1DArray.

// CHECK: OpCapability SparseResidency

// CHECK:      [[c12:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK:      [[c34:%[0-9]+]] = OpConstantComposite %v2int %int_3 %int_4
// CHECK:      [[c56:%[0-9]+]] = OpConstantComposite %v2int %int_5 %int_6
// CHECK:      [[c78:%[0-9]+]] = OpConstantComposite %v2int %int_7 %int_8
// CHECK:    [[c1to8:%[0-9]+]] = OpConstantComposite %_arr_v2int_uint_4 [[c12]] [[c34]] [[c56]] [[c78]]
// CHECK: [[cv4f_1_5:%[0-9]+]] = OpConstantComposite %v4float %float_1_5 %float_1_5 %float_1_5 %float_1_5

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4uint

float4 main(float3 location: A) : SV_Target {
// CHECK:            [[t2f4:%[0-9]+]] = OpLoad %type_2d_image_array %t2f4
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2f4]] [[gSampler]]
// CHECK-NEXT:        [[res:%[0-9]+]] = OpImageGather %v4float [[sampledImg]] [[loc]] %int_3 ConstOffset [[c12]]
// CHECK-NEXT:                       OpStore %a [[res]]
    float4 a = t2f4.GatherAlpha(gSampler, location, int2(1, 2));
// CHECK:            [[t2f4_0:%[0-9]+]] = OpLoad %type_2d_image_array %t2f4
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_0:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2f4_0]] [[gSampler_0]]
// CHECK-NEXT:        [[res_0:%[0-9]+]] = OpImageGather %v4float [[sampledImg_0]] [[loc_0]] %int_3 ConstOffsets [[c1to8]]
// CHECK-NEXT:                       OpStore %b [[res_0]]
    float4 b = t2f4.GatherAlpha(gSampler, location, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));

// CHECK:            [[t2u4:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t2u4
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_1:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u4]] [[gSampler_1]]
// CHECK-NEXT:        [[res_1:%[0-9]+]] = OpImageGather %v4uint [[sampledImg_1]] [[loc_1]] %int_3 ConstOffset [[c12]]
// CHECK-NEXT:                       OpStore %c [[res_1]]
    uint4 c = t2u4.GatherAlpha(gSampler, location, int2(1, 2));
// CHECK:            [[t2u4_0:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t2u4
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_2:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u4_0]] [[gSampler_2]]
// CHECK-NEXT:        [[res_2:%[0-9]+]] = OpImageGather %v4uint [[sampledImg_2]] [[loc_2]] %int_3 ConstOffsets [[c1to8]]
// CHECK-NEXT:                       OpStore %d [[res_2]]
    uint4 d = t2u4.GatherAlpha(gSampler, location, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8));

    uint status;
// CHECK:              [[t2u4_1:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t2u4
// CHECK-NEXT:     [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:          [[loc_3:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT:   [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u4_1]] [[gSampler_3]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseGather %SparseResidencyStruct [[sampledImg_3]] [[loc_3]] %int_3 ConstOffset [[c12]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult]] 1
// CHECK-NEXT:                         OpStore %e [[result]]
    uint4 e = t2u4.GatherAlpha(gSampler, location, int2(1, 2), status);

// CHECK:              [[t2u4_2:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t2u4
// CHECK-NEXT:     [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:          [[loc_4:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT:   [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2u4_2]] [[gSampler_4]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseGather %SparseResidencyStruct [[sampledImg_4]] [[loc_4]] %int_3 ConstOffsets [[c1to8]]
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %f [[result_0]]
    uint4 f = t2u4.GatherAlpha(gSampler, location, int2(1, 2), int2(3, 4), int2(5, 6), int2(7, 8), status);

// CHECK:        [[tCubeArray:%[0-9]+]] = OpLoad %type_cube_image_array %tCubeArray
// CHECK-NEXT:     [[gSampler_5:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_5:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[tCubeArray]] [[gSampler_5]]
// CHECK-NEXT: [[structResult_1:%[0-9]+]] = OpImageSparseGather %SparseResidencyStruct [[sampledImg_5]] [[cv4f_1_5]] %int_3 None
// CHECK-NEXT:       [[status_1:%[0-9]+]] = OpCompositeExtract %uint [[structResult_1]] 0
// CHECK-NEXT:                         OpStore %status [[status_1]]
// CHECK-NEXT:       [[result_1:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult_1]] 1
// CHECK-NEXT:                         OpStore %g [[result_1]]
    uint4 g = tCubeArray.GatherAlpha(gSampler, /*location*/ float4(1.5, 1.5, 1.5, 1.5), status);

    return 1.0;
}
