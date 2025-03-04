// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState gSampler : register(s5);

Texture1DArray   <float4> t1 : register(t1);
Texture2DArray   <float3> t2 : register(t2);
TextureCubeArray <float>  t3 : register(t3);

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_1
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_1
// CHECK: [[v4fc:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_1

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %float

float4 main(int2 offset: A, float comparator: B) : SV_Target {
// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg]] [[v2fc]] [[comparator]] ConstOffset %int_1
    float val1 = t1.SampleCmp(gSampler, float2(1, 1), comparator, 1);

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_0:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg_0]] [[v3fc]] [[comparator_0]] ConstOffset [[v2ic]]
    float val2 = t2.SampleCmp(gSampler, float3(1, 2, 1), comparator, 1);

// CHECK:              [[t3:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_1:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg_1]] [[v4fc]] [[comparator_1]]
    float val3 = t3.SampleCmp(gSampler, float4(1, 2, 3, 1), comparator);

    float clamp;
// CHECK:           [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:         [[t2_0:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_2:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2_0]] [[gSampler_2]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg_2]] [[v3fc]] [[comparator_2]] ConstOffset|MinLod [[v2ic]] [[clamp]]
    float val4 = t2.SampleCmp(gSampler, float3(1, 2, 1), comparator, 1, clamp);

// CHECK:              [[t3_0:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:   [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[comparator_3:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT: [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3_0]] [[gSampler_3]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg_3]] [[v4fc]] [[comparator_3]] MinLod %float_1_5
    float val5 = t3.SampleCmp(gSampler, float4(1, 2, 3, 1), comparator, /*clamp*/ 1.5);

    uint status;
// CHECK:             [[clamp_0:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:           [[t2_1:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:     [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[comparator_4:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:   [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2_1]] [[gSampler_4]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[sampledImg_4]] [[v3fc]] [[comparator_4]] ConstOffset|MinLod [[v2ic]] [[clamp_0]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %float [[structResult]] 1
// CHECK-NEXT:                         OpStore %val6 [[result]]
    float val6 = t2.SampleCmp(gSampler, float3(1, 2, 1), comparator, 1, clamp, status);

// CHECK:                [[t3_1:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:     [[gSampler_5:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[comparator_5:%[0-9]+]] = OpLoad %float %comparator
// CHECK-NEXT:   [[sampledImg_5:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3_1]] [[gSampler_5]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[sampledImg_5]] [[v4fc]] [[comparator_5]] MinLod %float_1_5
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val7 [[result_0]]
    float val7 = t3.SampleCmp(gSampler, float4(1, 2, 3, 1), comparator, /*clamp*/ 1.5, status);

    return 1.0;
}
