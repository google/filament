// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Used for ConstantBuffer definition.
// CHECK: [[type_CB_PerDraw:%[a-zA-Z0-9_]+]] = OpTypeStruct %mat4v4float
// Used for PerDraw local variable.
// CHECK: [[type_PerDraw:%[a-zA-Z0-9_]+]] = OpTypeStruct %mat4v4float

struct PerDraw {
  float4x4 Transform;
};

// CHECK: [[ptr_type_CB_PerDraw:%[a-zA-Z0-9_]+]] = OpTypePointer Uniform [[type_CB_PerDraw]]
// Used for ConstantBuffer to PerDraw type cast.


ConstantBuffer<PerDraw> PerDraws[];

struct VSInput {
  float3 Position : POSITION;

  [[vk::builtin("DrawIndex")]]
  uint DrawIdx    : DRAWIDX;
};

float4 main(in VSInput input) : SV_POSITION {
// CHECK:        [[ptr:%[a-zA-Z0-9_]+]] = OpAccessChain [[ptr_type_CB_PerDraw]] %PerDraws
// CHECK: [[cb_PerDraw:%[a-zA-Z0-9_]+]] = OpLoad [[type_CB_PerDraw]] [[ptr]]
// CHECK:   [[float4x4:%[a-zA-Z0-9_]+]] = OpCompositeExtract %mat4v4float [[cb_PerDraw]] 0
// CHECK: [[f_0_0:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 0 0
// CHECK: [[f_0_1:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 0 1
// CHECK: [[f_0_2:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 0 2
// CHECK: [[f_0_3:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 0 3
// CHECK: [[f_1_0:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 1 0
// CHECK: [[f_1_1:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 1 1
// CHECK: [[f_1_2:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 1 2
// CHECK: [[f_1_3:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 1 3
// CHECK: [[f_2_0:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 2 0
// CHECK: [[f_2_1:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 2 1
// CHECK: [[f_2_2:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 2 2
// CHECK: [[f_2_3:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 2 3
// CHECK: [[f_3_0:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 3 0
// CHECK: [[f_3_1:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 3 1
// CHECK: [[f_3_2:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 3 2
// CHECK: [[f_3_3:%[0-9]+]] = OpCompositeExtract %float [[float4x4]] 3 3
// CHECK: [[r0:%[0-9]+]] = OpCompositeConstruct %v4float [[f_0_0]] [[f_0_1]] [[f_0_2]] [[f_0_3]]
// CHECK: [[r1:%[0-9]+]] = OpCompositeConstruct %v4float [[f_1_0]] [[f_1_1]] [[f_1_2]] [[f_1_3]]
// CHECK: [[r2:%[0-9]+]] = OpCompositeConstruct %v4float [[f_2_0]] [[f_2_1]] [[f_2_2]] [[f_2_3]]
// CHECK: [[r3:%[0-9]+]] = OpCompositeConstruct %v4float [[f_3_0]] [[f_3_1]] [[f_3_2]] [[f_3_3]]
// CHECK: [[float4x4:%[0-9]+]] = OpCompositeConstruct %mat4v4float [[r0]] [[r1]] [[r2]] [[r3]]
// CHECK:                       OpCompositeConstruct [[type_PerDraw]] [[float4x4]]
  const PerDraw perDraw = PerDraws[input.DrawIdx];
  return mul(float4(input.Position, 1.0f), perDraw.Transform);
}
