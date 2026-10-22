// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

SamplerState           gSampler        : register(s4);
SamplerComparisonState gCompareSampler : register(s5);

// CHECK:               OpDecorate %t2f4 RelaxedPrecision
// CHECK-NEXT:          OpDecorate %t2i4 RelaxedPrecision
// CHECK-NEXT:          OpDecorate %t2f RelaxedPrecision
// CHECK-NEXT:          OpDecorate [[t2f4_val:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:          OpDecorate [[image_op_1:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:          OpDecorate [[t2i4_val:%[0-9]+]] RelaxedPrecision
// CHECK-NEXT:          OpDecorate [[image_op_2:%[0-9]+]] RelaxedPrecision
// CHECK:               OpDecorate [[t2f_val:%[0-9]+]] RelaxedPrecision
// CHECK:               OpDecorate [[imgSampleExplicit:%[0-9]+]] RelaxedPrecision
// CHECK:               OpDecorate [[t2f_val_again:%[0-9]+]] RelaxedPrecision
// CHECK:               OpDecorate [[imgSampleImplicit:%[0-9]+]] RelaxedPrecision

// CHECK:          %t2f4 = OpVariable %_ptr_UniformConstant_type_2d_image_array UniformConstant
// CHECK:          %t2i4 = OpVariable %_ptr_UniformConstant_type_2d_image UniformConstant
Texture2DArray<min16float4> t2f4   : register(t1);
Texture2D<min12int4>        t2i4   : register(t2);
Texture2D<min16float>       t2f    : register(t3);

float4 main(float3 location: A) : SV_Target {
// CHECK:   [[t2f4_val]] = OpLoad %type_2d_image_array %t2f4
// CHECK: [[image_op_1]] = OpImageGather %v4float
  float4 a = t2f4.GatherBlue(gSampler, location, int2(1, 2));

// CHECK:   [[t2i4_val]] = OpLoad %type_2d_image %t2i4
// CHECK: [[image_op_2]] = OpImageFetch %v4int
  int4 b = t2i4.Load(location, int2(1, 2));

// CHECK:           [[t2f_val]] = OpLoad %type_2d_image_0 %t2f
// CHECK: [[imgSampleExplicit]] = OpImageSampleDrefExplicitLod
  t2f.SampleCmpLevelZero(gCompareSampler, float2(0,0), 0);

// CHECK:     [[t2f_val_again]] = OpLoad %type_2d_image_0 %t2f
// CHECK: [[imgSampleImplicit]] = OpImageSampleImplicitLod
  t2f.Sample(gSampler, float2(0,0));

  return 1.0;
}

