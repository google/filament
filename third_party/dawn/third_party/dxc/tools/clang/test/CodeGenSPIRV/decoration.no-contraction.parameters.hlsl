// RUN: %dxc -T vs_6_0 -E VSMain %s -fcgl -spirv | FileCheck %s

struct VSInput
{
  float4 pos : ATTRIBUTE0;
};

float4x4 Proj;

precise float4 MakePrecise(precise float4 v) { return v; }

// CHECK: OpDecorate [[mul:%[0-9]+]] NoContraction

void VSMain(VSInput input, out precise float4 OutputPos : SV_Position)
{
  // CHECK: [[mul]] = OpMatrixTimesVector
  // CHECK: OpStore %param_var_v [[mul]]
  // CHECK: OpFunctionCall %v4float %MakePrecise %param_var_v
  OutputPos = MakePrecise(mul(input.pos, Proj));
}
