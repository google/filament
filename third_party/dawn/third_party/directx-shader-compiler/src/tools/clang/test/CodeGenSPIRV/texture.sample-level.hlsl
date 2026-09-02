// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState gSampler : register(s5);

// Note: The front end forbids sampling from non-floating-point texture formats.

Texture1D   <float4> t1 : register(t1);
Texture2D   <float4> t2 : register(t2);
Texture3D   <float4> t3 : register(t3);
TextureCube <float4> t4 : register(t4);
Texture3D   <float>  t5 : register(t5);
TextureCube <float2> t6 : register(t6);

// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_2 %int_2
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_3
// CHECK: [[v3ic:%[0-9]+]] = OpConstantComposite %v3int %int_2 %int_2 %int_2

// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image
// CHECK: %type_sampled_image_0 = OpTypeSampledImage %type_2d_image
// CHECK: %type_sampled_image_1 = OpTypeSampledImage %type_3d_image
// CHECK: %type_sampled_image_2 = OpTypeSampledImage %type_cube_image
// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float

float4 main(int3 offset: A) : SV_Target {
// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:   [[gSampler:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[gSampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleExplicitLod %v4float [[sampledImg]] %float_1 Lod %float_10
    float4 val1 = t1.SampleLevel(gSampler, 1, 10);

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:   [[gSampler_0:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_0:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t2]] [[gSampler_0]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleExplicitLod %v4float [[sampledImg_0]] [[v2fc]] Lod|ConstOffset %float_10 [[v2ic]]
    float4 val2 = t2.SampleLevel(gSampler, float2(1, 2), 10, 2);

// CHECK:              [[t3:%[0-9]+]] = OpLoad %type_3d_image %t3
// CHECK-NEXT:   [[gSampler_1:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_1:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3]] [[gSampler_1]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleExplicitLod %v4float [[sampledImg_1]] [[v3fc]] Lod|ConstOffset %float_10 [[v3ic]]
    float4 val3 = t3.SampleLevel(gSampler, float3(1, 2, 3), 10, 2);

// CHECK:              [[t4:%[0-9]+]] = OpLoad %type_cube_image %t4
// CHECK-NEXT:   [[gSampler_2:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT: [[sampledImg_2:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[t4]] [[gSampler_2]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleExplicitLod %v4float [[sampledImg_2]] [[v3fc]] Lod %float_10
    float4 val4 = t4.SampleLevel(gSampler, float3(1, 2, 3), 10);

    uint status;
// CHECK:                [[t3_0:%[0-9]+]] = OpLoad %type_3d_image %t3
// CHECK-NEXT:     [[gSampler_3:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_3:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t3_0]] [[gSampler_3]]
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[sampledImg_3]] [[v3fc]] Lod|ConstOffset %float_10 [[v3ic]]
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:                         OpStore %val5 [[result]]
    float4 val5 = t3.SampleLevel(gSampler, float3(1, 2, 3), 10, 2, status);

// CHECK:                [[t4_0:%[0-9]+]] = OpLoad %type_cube_image %t4
// CHECK-NEXT:     [[gSampler_4:%[0-9]+]] = OpLoad %type_sampler %gSampler
// CHECK-NEXT:   [[sampledImg_4:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[t4_0]] [[gSampler_4]]
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct [[sampledImg_4]] [[v3fc]] Lod %float_10
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:                         OpStore %val6 [[result_0]]
    float4 val6 = t4.SampleLevel(gSampler, float3(1, 2, 3), 10, status);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Make sure OpImageSampleExplicitLod returns a vec4.
// Make sure OpImageSparseSampleExplicitLod returns a struct, in which the second member is a vec4.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// CHECK: [[v4result:%[0-9]+]] = OpImageSampleExplicitLod %v4float {{%[0-9]+}} {{%[0-9]+}} Lod|ConstOffset %float_10 {{%[0-9]+}}
// CHECK:          {{%[0-9]+}} = OpCompositeExtract %float [[v4result]] 0
    float  val7 = t5.SampleLevel(gSampler, float3(1, 2, 3), 10, 2);

// CHECK: [[structResult_1:%[0-9]+]] = OpImageSparseSampleExplicitLod %SparseResidencyStruct {{%[0-9]+}} {{%[0-9]+}} Lod %float_10
// CHECK:     [[v4result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_1]] 1
// CHECK:              {{%[0-9]+}} = OpVectorShuffle %v2float [[v4result_0]] [[v4result_0]] 0 1
    float2 val8 = t6.SampleLevel(gSampler, float3(1, 2, 3), 10, status);

    return 1.0;
}
