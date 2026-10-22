// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_2d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 0 1 Unknown
// CHECK: [[type_2d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image]]

vk::SampledTexture2D<float4> tex2d;

void main() {
  float2 xy = float2(0.5, 0.5);

//CHECK:          [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
//CHECK-NEXT:    [[xy_load:%[a-zA-Z0-9_]+]] = OpLoad %v2float %xy
//CHECK-NEXT: [[query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex1]] [[xy_load]]
//CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[query]] 0
  float lod = tex2d.CalculateLevelOfDetail(xy);

}
