// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_75
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> tex;

float4 main() : SV_Target {
// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageSampleDrefExplicitLod %float [[tex0]] [[v3fc]] %float_0_25 Lod %float_0
  float a = tex.SampleCmpLevelZero(float3(0.5, 0.25, 0.75), 0.25f);

  uint status;
// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[b_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseSampleDrefExplicitLod %SparseResidencyStruct [[tex1]] [[v3fc]] %float_0_25 Lod %float_0
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[b_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float b = tex.SampleCmpLevelZero(float3(0.5, 0.25, 0.75), 0.25f, status);
  return float4(a + b, 0, 0, 1);
}
