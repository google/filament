// RUN: %dxc -T ps_6_2 -E main -spirv %s | FileCheck %s

// CHECK:                                       OpDecorate %in_var_A PerVertexKHR
// CHECK-DAG:                 %type_constants = OpTypeStruct %uint
// CHECK-DAG:    %_ptr_Uniform_type_constants = OpTypePointer Uniform %type_constants
// CHECK-DAG:            %_arr_v3float_uint_3 = OpTypeArray %v3float %uint_3
// CHECK-DAG: %_ptr_Input__arr_v3float_uint_3 = OpTypePointer Input %_arr_v3float_uint_3
cbuffer constants : register(b0)
{
  uint idx;
}

float4 main(nointerpolation float3 a : A) : SV_Target
{
// CHECK-DAG: %constants = OpVariable %_ptr_Uniform_type_constants Uniform
// CHECK-DAG:  %in_var_A = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
  float value = GetAttributeAtVertex(a, 0)[idx];
	return value.xxxx;
}
