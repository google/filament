// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_75
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> tex;

float4 main() : SV_Target {
// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageDrefGather %v4float [[tex0]] [[v3fc]] %float_0_25 None
  float4 a = tex.GatherCmpRed(float3(0.5, 0.25, 0.75), 0.25f);

  uint status;
// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[b_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseDrefGather %SparseResidencyStruct [[tex1]] [[v3fc]] %float_0_25 None
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[b_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float4 b = tex.GatherCmpRed(float3(0.5, 0.25, 0.75), 0.25f, status);

  return a + b;
}
