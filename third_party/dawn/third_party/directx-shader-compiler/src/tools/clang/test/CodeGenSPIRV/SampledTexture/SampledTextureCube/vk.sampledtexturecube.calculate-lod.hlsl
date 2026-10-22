// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> tex;

void main() {
  float3 xyz = float3(0.5, 0.25, 0.75);

// CHECK: [[tex_load:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[xyz_load:%[a-zA-Z0-9_]+]] = OpLoad %v3float %xyz
// CHECK: [[lod_query:%[a-zA-Z0-9_]+]] = OpImageQueryLod %v2float [[tex_load]] [[xyz_load]]
// CHECK: {{%[0-9]+}} = OpCompositeExtract %float [[lod_query]] 0
  float lod = tex.CalculateLevelOfDetail(xyz);
}
