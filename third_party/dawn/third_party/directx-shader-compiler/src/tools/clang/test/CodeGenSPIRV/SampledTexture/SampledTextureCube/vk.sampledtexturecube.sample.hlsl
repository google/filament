// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability MinLod
// CHECK: OpCapability SparseResidency

// CHECK: [[v3fc:%[0-9]+]] = OpConstantComposite %v3float %float_0_5 %float_0_25 %float_0_75
// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> tex;

float4 main() : SV_Target {
// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[a:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex0]] [[v3fc]] None
  float4 a = tex.Sample(float3(0.5, 0.25, 0.75));

// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[b:%[a-zA-Z0-9_]+]] = OpImageSampleImplicitLod %v4float [[tex1]] [[v3fc]] MinLod %float_0_5
  float4 b = tex.Sample(float3(0.5, 0.25, 0.75), 0.5f);

  uint status;
// CHECK: [[tex2:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[c_sparse:%[a-zA-Z0-9_]+]] = OpImageSparseSampleImplicitLod %SparseResidencyStruct [[tex2]] [[v3fc]] MinLod %float_0_5
// CHECK: [[status0:%[a-zA-Z0-9_]+]] = OpCompositeExtract %uint [[c_sparse]] 0
// CHECK: OpStore %status [[status0]]
  float4 c = tex.Sample(float3(0.5, 0.25, 0.75), 0.5f, status);
  return a + b + c;
}
