// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v4fc:%[0-9]+]] = OpConstantComposite %v4float %float_0_5 %float_0_25 %float_0_75 %float_1
// CHECK: [[type_cube_array_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 1 0 1 Unknown
// CHECK: [[type_cube_array_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_array_image]]

vk::SampledTextureCUBEArray<float4> texf4;

float4 main() : SV_Target {
// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %texf4
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageGather %v4float [[tex0]] [[v4fc]] %int_2 None
  float4 a = texf4.GatherBlue(float4(0.5, 0.25, 0.75, 1.0));

  uint status;
// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %texf4
// CHECK: [[b_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseGather %SparseResidencyStruct [[tex1]] [[v4fc]] %int_2 None
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[b_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float4 b = texf4.GatherBlue(float4(0.5, 0.25, 0.75, 1.0), status);

  return a + b;
}
