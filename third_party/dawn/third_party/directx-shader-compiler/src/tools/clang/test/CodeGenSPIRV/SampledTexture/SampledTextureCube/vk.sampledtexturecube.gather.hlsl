// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_75
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]
// CHECK: [[type_cube_image_uint:%[a-zA-Z0-9_]+]] = OpTypeImage %uint Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled_uint:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image_uint]]

vk::SampledTextureCUBE<float4> texf4;
vk::SampledTextureCUBE<uint> texu;

float4 main() : SV_Target {
// CHECK: [[texf4_0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %texf4
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[texf4_0]] [[v3fc]] %int_0 None
  float4 a = texf4.Gather(float3(0.5, 0.25, 0.75));

// CHECK: [[texu_0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled_uint]] %texu
// CHECK: [[b:%[a-zA-Z0-9_]+]] = OpImageGather %v4uint [[texu_0]] [[v3fc]] %int_0 None
  uint4 b = texu.Gather(float3(0.5, 0.25, 0.75));

  uint status;
// CHECK: [[texf4_1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %texf4
// CHECK: [[c_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[texf4_1]] [[v3fc]] %int_0 None
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[c_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float4 c = texf4.Gather(float3(0.5, 0.25, 0.75), status);

  return a + c + float4(b);
}
