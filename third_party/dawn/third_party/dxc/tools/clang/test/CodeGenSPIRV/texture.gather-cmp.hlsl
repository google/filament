// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState gSampler : register(s5);

Texture2D<float4> t1 : register(t1);
Texture2D<float2> t2 : register(t2);
Texture2D<float>  t3 : register(t3);
TextureCube<float>t4 : register(t4);
// .GatherCmp() does not support Texture1D and Texture3D.

// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_2
// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1_5 %float_1_5 %float_1_5

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float

float4 main(float2 location: A, float comparator: B, int2 offset: C) : SV_Target {
// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image %t1
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageDrefGather %v4float [[sampledImg]] [[loc]] [[comparator]] ConstOffset [[v2ic]]
    float4 val1 = t1.GatherCmp(gSampler, location, comparator, int2(1, 2));

// CHECK:              [[t2_0:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_0:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image [[t2_0]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageDrefGather %v4float [[sampledImg_0]] [[v2fc]] [[comparator_0]]
    float4 val2 = t2.GatherCmp(gSampler, float2(1, 2), comparator);

// CHECK:              [[t3:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:        [[loc_0:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT: [[comparator_1:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:     [[offset:%[0-9]+]] = OpLoad %v2int %offset
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image [[t3]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageDrefGather %v4float [[sampledImg_1]] [[loc_0]] [[comparator_1]] Offset [[offset]]
    float4 val3 = t3.GatherCmp(gSampler, location, comparator, offset);

    uint status;

// CHECK:                [[t3_0:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:     [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:          [[loc_1:%[0-9]+]] = OpLoad %v2float %location
// CHECK-NEXT:   [[comparator_2:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:       [[offset_0:%[0-9]+]] = OpLoad %v2int %offset
// CHECK-NEXT:   [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image [[t3_0]] [[gSampler_2]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[sampledImg_2]] [[loc_1]] [[comparator_2]] Offset [[offset_0]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                         OpStore %val4 [[result]]
    float4 val4 = t3.GatherCmp(gSampler, location, comparator, offset, status);

// CHECK:                [[t4:%[0-9]+]] = OpLoad %type_cube_image %t4
// CHECK-NEXT:     [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[comparator_3:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:   [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t4]] [[gSampler_3]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[sampledImg_3]] [[v3fc]] [[comparator_3]] None
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val5 [[result_0]]
    float4 val5 = t4.GatherCmp(gSampler, /*location*/float3(1.5, 1.5, 1.5), comparator, status);

    return 1.0;
}
