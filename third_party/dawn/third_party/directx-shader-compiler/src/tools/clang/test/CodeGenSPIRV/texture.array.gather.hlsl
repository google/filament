// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState gSampler : register(s1);

Texture2DArray   <float4> t2 : register(t2);
TextureCubeArray <uint3>  t4 : register(t4);
Texture2DArray   <int3>   t6 : register(t6);
TextureCubeArray <float>  t8 : register(t8);
// .Gather() does not support Texture1DArray.

// CHECK: OpCapability ImageGatherExtended
// CHECK: OpCapability SparseResidency

// CHECK: [[v4fc:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4int
// CHECK: %SparseResidencyStruct_0 = OpTypeStruct %uint %v4float

float4 main(float3 location: A, int2 offset: B) : SV_Target {

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT:     [[offset:%[0-9]+]] = OpLoad %v2int %offset
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageGather %v4float [[sampledImg]] [[loc]] %int_0 Offset [[offset]]
    float4 val2 = t2.Gather(gSampler, location, offset);

// CHECK:              [[t4:%[0-9]+]] = OpLoad %type_cube_image_array %t4
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t4]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageGather %v4uint [[sampledImg_0]] [[v4fc]] %int_0
    uint4 val4 = t4.Gather(gSampler, float4(1, 2, 3, 4));

// CHECK:              [[t6:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t6
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_0:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT:     [[offset_0:%[0-9]+]] = OpLoad %v2int %offset
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t6]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageGather %v4int [[sampledImg_1]] [[loc_0]] %int_0 Offset [[offset_0]]
    int4 val6 = t6.Gather(gSampler, location, offset);

// CHECK:              [[t8:%[0-9]+]] = OpLoad %type_cube_image_array_0 %t8
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[t8]] [[gSampler_2]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageGather %v4float [[sampledImg_2]] [[v4fc]] %int_0
    float4 val8 = t8.Gather(gSampler, float4(1, 2, 3, 4));

    uint status;
// CHECK:                [[t6_0:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t6
// CHECK-NEXT:     [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:          [[loc_1:%[0-9]+]] = OpLoad %v3float %location
// CHECK-NEXT:       [[offset_1:%[0-9]+]] = OpLoad %v2int %offset
// CHECK-NEXT:   [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t6_0]] [[gSampler_3]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseGather %SparseResidencyStruct [[sampledImg_3]] [[loc_1]] %int_0 Offset [[offset_1]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %v4int [[structResult]] 1
// CHECK-NEXT:                         OpStore %val9 [[result]]
    int4 val9 = t6.Gather(gSampler, location, offset, status);

// CHECK:                [[t8_0:%[0-9]+]] = OpLoad %type_cube_image_array_0 %t8
// CHECK-NEXT:     [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[t8_0]] [[gSampler_4]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseGather %SparseResidencyStruct_0 [[sampledImg_4]] [[v4fc]] %int_0 None
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val10 [[result_0]]
    float4 val10 = t8.Gather(gSampler, float4(1, 2, 3, 4), status);

    return 1.0;
}

