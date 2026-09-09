// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState gSampler : register(s5);

Texture1D   <float4> t1 : register(t1);
Texture2D   <float2> t2 : register(t2);
TextureCube <float>  t4 : register(t4);
// No .SampleCmp() for Texture3D.

// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_1
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_3

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %float

float4 main(int2 offset: A, float comparator: B) : SV_Target {
// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg]] %float_1 [[comparator]] Lod|ConstOffset %float_0 %int_5
    float val1 = t1.SampleCmpLevelZero(gSampler, 1, comparator, 5);

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_0:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg_0]] [[v2fc]] [[comparator_0]] Lod|ConstOffset %float_0 [[v2ic]]
    float val2 = t2.SampleCmpLevelZero(gSampler, float2(1, 2), comparator, 1);

// CHECK:              [[t4:%[0-9]+]] = OpLoad %type_cube_image %t4
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_1:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t4]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg_1]] [[v3fc]] [[comparator_1]] Lod %float_0
    float val4 = t4.SampleCmpLevelZero(gSampler, float3(1, 2, 3), comparator);

    uint status;
// CHECK:                [[t2_0:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:     [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[comparator_2:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:   [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2_0]] [[gSampler_2]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImg_2]] [[v2fc]] [[comparator_2]] Lod|ConstOffset %float_0 [[v2ic]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %float [[structResult]] 1
// CHECK-NEXT:                         OpStore %val5 [[result]]
    float val5 = t2.SampleCmpLevelZero(gSampler, float2(1, 2), comparator, 1, status);

// CHECK:                [[t4_0:%[0-9]+]] = OpLoad %type_cube_image %t4
// CHECK-NEXT:     [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[comparator_3:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:   [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t4_0]] [[gSampler_3]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImg_3]] [[v3fc]] [[comparator_3]] Lod %float_0
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val6 [[result_0]]
    float val6 = t4.SampleCmpLevelZero(gSampler, float3(1, 2, 3), comparator, status);

    return 1.0;
}
