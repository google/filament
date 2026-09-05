// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_75
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> texf4;

float4 main() : SV_Target {
// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %texf4
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex0]] [[v3fc]] %int_1 None
  float4 a = texf4.GatherGreen(float3(0.5, 0.25, 0.75));

  uint status;
// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %texf4
// CHECK: [[b_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[tex1]] [[v3fc]] %int_1 None
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[b_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float4 b = texf4.GatherGreen(float3(0.5, 0.25, 0.75), status);

  return a + b;
}
