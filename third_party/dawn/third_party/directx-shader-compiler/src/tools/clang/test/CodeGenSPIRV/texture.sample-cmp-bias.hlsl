// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState s;

Texture1D<float4> t1;
Texture1DArray<float4> t1_array;
Texture2D<float4> t2;
TextureCube<float4> tcube;

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_n5 %int_7
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_3

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %float

void main() {
    float cmpVal;
    float bias;

    float clamp;

// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:       [[bias:%[0-9]+]] = OpLoad %float %bias
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg]] %float_1 [[cmpVal]] Bias [[bias]]
    float val1 = t1.SampleCmpBias(s, 1, cmpVal, bias);

// CHECK:         [[t1Array:%[0-9]+]] = OpLoad %type_1d_image_array %t1_array
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:       [[bias:%[0-9]+]] = OpLoad %float %bias
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t1Array]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg]] [[v2fc]] [[cmpVal]] Bias|ConstOffset [[bias]] %int_n5
    float val2 = t1_array.SampleCmpBias(s, float2(1, 2), cmpVal, bias, -5);

// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:       [[bias:%[0-9]+]] = OpLoad %float %bias
// CHECK-NEXT:      [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t2]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefImplicitLod %float [[sampledImg]] [[v2fc]] [[cmpVal]] Bias|ConstOffset|MinLod [[bias]] [[v2ic]] [[clamp]]
    float val3 = t2.SampleCmpBias(s, float2(1, 2), cmpVal, bias, uint2(-5, 7), clamp);

    uint status;

// CHECK:              [[tcube:%[0-9]+]] = OpLoad %type_cube_image %tcube
// CHECK-NEXT:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:        [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:       [[bias:%[0-9]+]] = OpLoad %float %bias
// CHECK-NEXT:         [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:    [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[tcube]] [[sampler]]
// CHECK-NEXT: [[structResult:%[0-9]+]]  = OpImageSparseSampleDrefImplicitLod %SparseResidencyStruct [[sampledImg]] [[v3fc]] [[cmpVal]] Bias|MinLod [[bias]] [[clamp]]
// CHECK-NEXT:        [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                             OpStore %status [[status]]
// CHECK-NEXT:        [[result:%[0-9]+]] = OpCompositeExtract %float [[structResult]] 1
// CHECK-NEXT:                             OpStore %val4 [[result]]
    float val4 = tcube.SampleCmpBias(s, float3(1, 2, 3), cmpVal, bias, clamp, status);
}