// RUN: %dxc -T ps_6_8 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_3d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image]]

vk::SampledTexture3D<float4> tex3d;

void main() {
  float3 xyz = float3(0.5, 0.5, 0.5);

// CHECK:          [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT: [[xyz_load:%[a-zA-Z0-9_]+]] = OpLoad %v3float %xyz
// CHECK-NEXT: [[query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex1_load]] [[xyz_load]]
// CHECK-NEXT:        {{%[0-9]+}} = OpCompositeExtract %float [[query]] 1
  float lod1 = tex3d.CalculateLevelOfDetailUnclamped(xyz);
}
