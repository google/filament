// RUN: %dxc -T ps_6_0 -E main -fcgl -Vd -spirv -fcgl  %s -spirv | FileCheck %s

[[vk::ext_decorate(1, 0)]]
bool b0;

int getAlignment() {
  return 16;
}

//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} SpecId 0
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordNoPerspAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordNoPerspCentroidAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordNoPerspSampleAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordSmoothAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordSmoothCentroidAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordSmoothSampleAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} BuiltIn BaryCoordPullModelAMD
//CHECK: OpDecorate {{%[a-zA-Z0-9_]+}} ExplicitInterpAMD

//CHECK-DAG: OpDecorate {{%[a-zA-Z0-9_]+}} Location 23
//CHECK-DAG: OpDecorateString {{%[a-zA-Z0-9_]+}} UserSemantic "return variable"
//CHECK-DAG: OpDecorate {{%[a-zA-Z0-9_]+}} FPRoundingMode RTE

//CHECK-DAG: OpDecorateId {{%[a-zA-Z0-9_]+}} AlignmentId [[alignment:%[a-zA-Z0-9_]+]]
//CHECK-DAG: OpDecorateId {{%[a-zA-Z0-9_]+}} UniformId %int_13
//CHECK-DAG: [[alignment]] = OpFunctionCall %int %getAlignment

[[vk::ext_decorate(30, 23)]]
float4 main(
// spv::Decoration::builtIn = 11
[[vk::ext_decorate(11, 4992)]] float2 b0 : COLOR0,
[[vk::ext_decorate(11, 4993)]] float2 b1 : COLOR1,
[[vk::ext_decorate(11, 4994)]] float2 b2 : COLOR2,
[[vk::ext_decorate(11, 4995)]] float2 b3 : COLOR3,
[[vk::ext_decorate(11, 4996)]] float2 b4 : COLOR4,
[[vk::ext_decorate(11, 4997)]] float2 b5 : COLOR5,
[[vk::ext_decorate(11, 4998)]] float2 b6 : COLOR6,
// ExplicitInterpAMD
[[vk::location(12), vk::ext_decorate(4999)]] float2 b7 : COLOR7,
[[vk::location(13), vk::ext_decorate_id(/* AlignmentId */ 46, getAlignment())]]
int foo : FOO
) : SV_Target {
  [[vk::ext_decorate_id(/* UniformId */ 27, 13)]]
  int bar;

  // spv::Decoration::FPRoundingMode = 39, RTZ=0
  [[vk::ext_decorate(39, 0), vk::ext_decorate_string(5635, "return variable")]] float4 ret = 1.0;
  ret.xy = b0 * b1 + b2 + b3 + b4;
  ret.zw = b5 + b6 + b7;
  return ret;
}
