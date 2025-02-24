// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState gSampler : register(s5);

// Note: The front end forbids sampling from non-floating-point texture formats.

Texture1DArray   <float4> t1 : register(t1);
Texture2DArray   <float4> t2 : register(t2);
TextureCubeArray <float4> t3 : register(t3);
Texture2DArray   <float>  t4 : register(t4);
TextureCubeArray <float3> t5 : register(t5);

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_1
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_1
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_1 %int_1
// CHECK: [[v4fc:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_1

// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array
// CHECK: %type_sampled_image_0 = OpTypeSampledImage %type_2d_image_array
// CHECK: %type_sampled_image_1 = OpTypeSampledImage %type_cube_image_array
// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float

float4 main(int2 offset : A) : SV_Target {
// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg]] [[v2fc]] Bias|ConstOffset %float_0_5 %int_1
    float4 val1 = t1.SampleBias(gSampler, float2(1, 1), 0.5, 1);

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image_array %t2
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg_0]] [[v3fc]] Bias|ConstOffset %float_0_5 [[v2ic]]
    float4 val2 = t2.SampleBias(gSampler, float3(1, 2, 1), 0.5, 1);

// CHECK:              [[t3:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg_1]] [[v4fc]] Bias %float_0_5
    float4 val3 = t3.SampleBias(gSampler, float4(1, 2, 3, 1), 0.5);

    float clamp;
// CHECK:           [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:         [[t1_0:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1_0]] [[gSampler_2]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg_2]] [[v2fc]] Bias|ConstOffset|MinLod %float_0_5 %int_1 [[clamp]]
    float4 val4 = t1.SampleBias(gSampler, float2(1, 1), 0.5, 1, clamp);

// CHECK:              [[t3_0:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:   [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3_0]] [[gSampler_3]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleImplicitLod %v4float [[sampledImg_3]] [[v4fc]] Bias|MinLod %float_0_5 %float_2_5
    float4 val5 = t3.SampleBias(gSampler, float4(1, 2, 3, 1), 0.5, /*clamp*/ 2.5f);

    uint status;
// CHECK:             [[clamp_0:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:           [[t1_1:%[0-9]+]] = OpLoad %type_1d_image_array %t1
// CHECK-NEXT:     [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1_1]] [[gSampler_4]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[sampledImg_4]] [[v2fc]] Bias|ConstOffset|MinLod %float_0_5 %int_1 [[clamp_0]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                         OpStore %val6 [[result]]
    float4 val6 = t1.SampleBias(gSampler, float2(1, 1), 0.5, 1, clamp, status);

// CHECK:                [[t3_1:%[0-9]+]] = OpLoad %type_cube_image_array %t3
// CHECK-NEXT:     [[gSampler_5:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_5:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3_1]] [[gSampler_5]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[sampledImg_5]] [[v4fc]] Bias|MinLod %float_0_5 %float_2_5
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val7 [[result_0]]
    float4 val7 = t3.SampleBias(gSampler, float4(1, 2, 3, 1), 0.5, /*clamp*/ 2.5f, status);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Make sure OpImageSampleImplicitLod returns a vec4.
// Make sure OpImageSparseSampleImplicitLod returns a struct, in which the second member is a vec4.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// CHECK: [[v4result:%[0-9]+]] = OpImageSampleImplicitLod %v4float {{%[0-9]+}} {{%[0-9]+}} Bias|ConstOffset %float_0_5 {{%[a-zA-Z0-9_]+}}
// CHECK:           {{%[0-9]+}} = OpCompositeExtract %float [[v4result]] 0
    float  val8 = t4.SampleBias(gSampler, float3(1, 2, 1), 0.5, 1);

// CHECK: [[structResult_1:%[0-9]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct {{%[0-9]+}} {{%[0-9]+}} Bias|MinLod %float_0_5 %float_2_5
// CHECK:     [[v4result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_1]] 1
// CHECK:              {{%[0-9]+}} = OpVectorShuffle %v3float [[v4result_0]] [[v4result_0]] 0 1 2
    float3 val9 = t5.SampleBias(gSampler, float4(1, 2, 3, 1), 0.5, /*clamp*/ 2.5f, status);

    return 1.0;
}
