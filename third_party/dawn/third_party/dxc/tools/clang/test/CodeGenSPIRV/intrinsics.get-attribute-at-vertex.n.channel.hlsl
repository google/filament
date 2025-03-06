// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate [[in_var_COLOR0:%[a-zA-Z0-9_]+]] Location 0
// CHECK: OpDecorate [[in_var_COLOR1:%[a-zA-Z0-9_]+]] Location 1
// CHECK: OpDecorate [[in_var_COLOR2:%[a-zA-Z0-9_]+]] Location 2
// CHECK: OpDecorate [[in_var_COLOR1]] PerVertexKHR
// CHECK: OpDecorate [[in_var_COLOR2]] PerVertexKHR
// CHECK: [[in_var_COLOR0]] = OpVariable %_ptr_Input_v3float Input
// CHECK: [[in_var_COLOR1]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[in_var_COLOR2]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
float3 main(
    float3 color0 : COLOR0,
    nointerpolation float3 color1 : COLOR1,
    nointerpolation float3 color2 : COLOR2) : SV_Target
{
    float3 vColor0 = color0;
    float3 vColor1 = color1;
    float3 vColor2 = GetAttributeAtVertex( color1, VertexID::SECOND );
    float3 vColor3 = color1;
    float3 vColor4 = float3(color2.y, color1.z, vColor2.y);
    return vColor0 + vColor1 + vColor2 + vColor3 + vColor4;
}

// CHECK: [[color0:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function_v3float
// CHECK: [[color1:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function__arr_v3float_uint_3
// CHECK: [[color2:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function__arr_v3float_uint_3
// CHECK: [[inst104:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[color2]] %uint_0 %int_1
// CHECK: [[inst105:%[0-9]+]] = OpLoad %float [[inst104]]
// CHECK: [[inst106:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[color1]] %uint_0 %int_2
// CHECK: [[inst107:%[0-9]+]] = OpLoad %float [[inst106]]
// CHECK: [[inst108:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[vColor2:%[a-zA-Z0-9_]+]] %int_1
// CHECK: [[inst109:%[0-9]+]] = OpLoad %float [[inst108]]
// CHECK: [[inst110:%[0-9]+]] = OpCompositeConstruct %v3float [[inst105]] [[inst107]] [[inst109]]
// CHECK: OpStore [[vColor4:%[a-zA-Z0-9_]+]] [[inst110]]