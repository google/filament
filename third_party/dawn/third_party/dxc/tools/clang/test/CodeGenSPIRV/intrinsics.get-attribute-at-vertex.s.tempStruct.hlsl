// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};


struct PSInput
{
    float2 m0 : COLOR0;
    nointerpolation float3 m1 : COLOR1;
    nointerpolation float3 m2 : COLOR2;
};

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate [[color4:%[a-zA-Z0-9_]+]] Location 1
// CHECK: OpDecorate [[color5:%[a-zA-Z0-9_]+]] Location 2
// CHECK: OpDecorate [[color4]] PerVertexKHR
// CHECK: OpDecorate [[color5]] PerVertexKHR
// CHECK: [[PSInput:%[a-zA-Z0-9_]+]] = OpTypeStruct %v2float %_arr_v3float_uint_3 %_arr_v3float_uint_3
// CHECK: [[color4]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[color5]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[outTarget1:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Output_v3float Output
// CHECK: [[outTarget2:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Output_v3float Output
PSInput main( float2 color0 : COLOR3,
             nointerpolation float3 color1 : COLOR4,
             nointerpolation float3 color2 : COLOR5) : SV_Target
{
    PSInput retVal;
    retVal.m0 = color0;
    retVal.m1 += color1;
    retVal.m1 += GetAttributeAtVertex( color1, VertexID::SECOND );
    retVal.m2 = color2;
    retVal.m2 = GetAttributeAtVertex( color2, VertexID::THIRD );
    return retVal;
}

// CHECK: [[c1pvt0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_v3float Function
// CHECK: [[c2pvt0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_v3float Function
// CHECK: [[c1e0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[funcp1:%[a-zA-Z0-9_]+]] %uint_0
// CHECK: [[c2e0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[funcp2:%[a-zA-Z0-9_]+]] %uint_0
// CHECK: [[loadc2e0:%[0-9]+]] = OpLoad %v3float [[c2e0]]
// CHECK: OpStore [[c2pvt0]] [[loadc2e0]]
// CHECK: [[ldc2pvt0:%[0-9]+]] = OpLoad %v3float [[c2pvt0]]
// CHECK: [[acRetVal2:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[retVal:%[a-zA-Z0-9_]+]] %int_2 %uint_0
// CHECK: OpStore [[acRetVal2]] [[ldc2pvt0]]