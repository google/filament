// RUN: %dxc -T vs_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

template<typename T>
T test(const T a, const T b = 0)
{
  return a + b;
}

float4 main(uint vertex_id : SV_VertexID) : SV_Position
{
  // CHECK: OpStore %param_var_a %float_1
  // CHECK: OpStore %param_var_b %float_2
  // CHECK: [[first:%[0-9]+]] = OpFunctionCall %float %test %param_var_a %param_var_b
  // CHECK: OpStore %param_var_a_0 %float_4
  // CHECK: OpStore %param_var_b_0 %float_0
  // CHECK: [[second:%[0-9]+]] = OpFunctionCall %float %test %param_var_a_0 %param_var_b_0
  // CHECK: {{%[0-9]+}} = OpCompositeConstruct %v4float [[first]] [[second]] %float_0 %float_0
  return float4(test<float>(1,2), test<float>(4), 0, 0);
}
