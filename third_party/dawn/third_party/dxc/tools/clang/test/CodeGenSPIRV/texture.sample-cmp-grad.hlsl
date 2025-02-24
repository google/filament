// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

SamplerComparisonState s;

Texture1D<float4> t1;
Texture1DArray<float4> t1_array;
Texture2D<float4> t2;
TextureCube<float4> tcube;

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v2fc:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_2
// CHECK: [[v2f_2:%[0-9]+]] = OpConstantComposite %v2float %float_2 %float_2
// CHECK: [[v2f_3:%[0-9]+]] = OpConstantComposite %v2float %float_3 %float_3
// CHECK: [[v2ic:%[0-9]+]] = OpConstantComposite %v2int %int_n5 %int_7
// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_2 %float_3
// CHECK: [[v3f_1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v3f_2:%[0-9]+]] = OpConstantComposite %v3float %float_2 %float_2 %float_2


// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %float

void main() {
    float cmpVal;

    float clamp;

// CHECK:              [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg]] %float_1 [[cmpVal]] Grad %float_2 %float_3
    float val1 = t1.SampleCmpGrad(s, 1, cmpVal, 2, 3);

// CHECK:         [[t1Array:%[0-9]+]] = OpLoad %type_1d_image_array %t1_array
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_0 [[t1Array]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg]] [[v2fc]] [[cmpVal]] Grad|ConstOffset %float_2 %float_3 %int_n5
    float val2 = t1_array.SampleCmpGrad(s, float2(1, 2), cmpVal, 2, 3, -5);


// CHECK:              [[t2:%[0-9]+]] = OpLoad %type_2d_image %t2
// CHECK-NEXT:    [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:     [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:      [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT: [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_1 [[t2]] [[sampler]]
// CHECK-NEXT:            {{%[0-9]+}} = OpImageSampleDrefExplicitLod %float [[sampledImg]] [[v2fc]] [[cmpVal]] Grad|ConstOffset|MinLod [[v2f_2]] [[v2f_3]] [[v2ic]] [[clamp]]
    float val3 = t2.SampleCmpGrad(s, float2(1, 2), cmpVal, float2(2, 2), float2(3, 3), uint2(-5, 7), clamp);

    uint status;

// CHECK:              [[tcube:%[0-9]+]] = OpLoad %type_cube_image %tcube
// CHECK-NEXT:       [[sampler:%[0-9]+]] = OpLoad %type_sampler %s
// CHECK-NEXT:        [[cmpVal:%[0-9]+]] = OpLoad %float %cmpVal
// CHECK-NEXT:         [[clamp:%[0-9]+]] = OpLoad %float %clamp
// CHECK-NEXT:    [[sampledImg:%[0-9]+]] = OpSampledImage %type_sampled_image_2 [[tcube]] [[sampler]]
// CHECK-NEXT: [[structResult:%[0-9]+]]  = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[sampledImg]] [[v3fc]] [[cmpVal]] Grad|MinLod [[v3f_1]] [[v3f_2]] [[clamp]]
// CHECK-NEXT:        [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                             OpStore %status [[status]]
// CHECK-NEXT:        [[result:%[0-9]+]] = OpCompositeExtract %float [[structResult]] 1
// CHECK-NEXT:                             OpStore %val4 [[result]]
    float val4 = tcube.SampleCmpGrad(s, float3(1, 2, 3), cmpVal, float3(1, 1, 1), float3(2, 2, 2), clamp, status);
}