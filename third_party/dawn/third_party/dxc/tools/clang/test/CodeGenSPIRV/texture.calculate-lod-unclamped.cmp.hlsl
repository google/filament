// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

SamplerComparisonState scs : register(s2);

Texture1D        <float>  t1;

// CHECK:   %type_sampled_image = OpTypeSampledImage %type_1d_image

void main() {
  float x = 0.5;

//CHECK:          [[t1:%[0-9]+]] = OpLoad %type_1d_image %t1
//CHECK-NEXT:    [[ss1:%[0-9]+]] = OpLoad %type_sampler %scs
//CHECK-NEXT:     [[x1:%[0-9]+]] = OpLoad %float %x
//CHECK-NEXT:    [[si1:%[0-9]+]] = OpSampledImage %type_sampled_image [[t1]] [[ss1]]
//CHECK-NEXT: [[query1:%[0-9]+]] = OpImageQueryLod %v2float [[si1]] [[x1]]
//CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[query1]] 1
  float lod1 = t1.CalculateLevelOfDetailUnclamped(scs, x);
}
