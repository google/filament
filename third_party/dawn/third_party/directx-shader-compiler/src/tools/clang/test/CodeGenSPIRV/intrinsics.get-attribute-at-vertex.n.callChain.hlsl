// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID {
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};


float3 func1(nointerpolation float3 func1Color){
  float3 func1Ret = GetAttributeAtVertex(func1Color, 1) + func1Color;
  return func1Ret;
}

float3 func0(nointerpolation float3 func0Color){
  return func1(func0Color);
}

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate %in_var_COLOR Location 0
// CHECK: OpDecorate %in_var_COLOR PerVertexKHR
// CHECK: %in_var_COLOR = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
float3 main(nointerpolation float3 inputColor : COLOR) : SV_Target
{
    float3 mainRet = func0(inputColor);
    return mainRet;
}
// CHECK: [[param_var_inputColor:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function__arr_v3float_uint_3 Function
// CHECK: [[param_var_func0Color:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function__arr_v3float_uint_3 Function
// CHECK: [[inputColor_perVertexParam0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_v3float Function
// CHECK: [[inst36:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[param_var_func0Color]] %uint_0
// CHECK: [[inst37:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[inputColor:%[a-zA-Z0-9_]+]] %uint_0
// CHECK: [[inst38:%[0-9]+]] = OpLoad %v3float [[inst37]]
// CHECK: OpStore [[inputColor_perVertexParam0]] [[inst38]]
// CHECK: [[func1Color:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function__arr_v3float_uint_3
// CHECK: [[func1Color_perVertexParam0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_v3float Function
// CHECK: [[inst59:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[func1Color]] %uint_0
// CHECK: [[inst60:%[0-9]+]] = OpLoad %v3float [[inst59]]
// CHECK: OpStore [[func1Color_perVertexParam0]] [[inst60]]