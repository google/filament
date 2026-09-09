// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_2d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 0 1 Unknown
// CHECK: [[type_2d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_array]]

vk::SampledTexture2DArray<float4> tex2darray;

void main() {
  float2 xy = float2(0.5, 0.5);

//CHECK:          [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_2d_sampled_image_array]] %tex2darray
//CHECK-NEXT:    [[xy_load:%[a-zA-Z0-9_]+]] = OpLoad %v2float %xy
//CHECK-NEXT: [[query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex1]] [[xy_load]]
//CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[query]] 0
  float lod = tex2darray.CalculateLevelOfDetail(xy);

}
