// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};


struct PSInput
{
    nointerpolation float3 color0 : COLOR0;
    nointerpolation float3 color1 : COLOR1;
    nointerpolation float3 color2 : COLOR2;
};

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate [[color0:%[a-zA-Z0-9_]+]] Location 0
// CHECK: OpDecorate [[color1:%[a-zA-Z0-9_]+]] Location 1
// CHECK: OpDecorate [[color2:%[a-zA-Z0-9_]+]] Location 2
// CHECK: OpDecorate [[color0]] PerVertexKHR
// CHECK: OpDecorate [[color1]] PerVertexKHR
// CHECK: OpDecorate [[color2]] PerVertexKHR
// CHECK: [[PSInput:%[a-zA-Z0-9_]+]] = OpTypeStruct %_arr_v3float_uint_3 %_arr_v3float_uint_3 %_arr_v3float_uint_3
// CHECK: [[color0]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[color1]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[color2]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[outTarget0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Output_v3float Output
// CHECK: [[outTarget1:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Output_v3float Output
// CHECK: [[outTarget2:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Output_v3float Output
PSInput main( PSInput input ) : SV_Target
{
    PSInput retVal;
    retVal.color0 = 0.1;
    retVal.color1 += input.color1;
    retVal.color1 += GetAttributeAtVertex( input.color1, VertexID::SECOND );
    retVal.color2 = input.color2;
    retVal.color2 = GetAttributeAtVertex( input.color2, VertexID::THIRD );
    return retVal;
}

// CHECK: [[input:%[a-zA-Z0-9_]+]] = OpFunctionParameter [[Ptr_PSInput:%[a-zA-Z0-9_]+]]
// CHECK: [[retVal:%[a-zA-Z0-9_]+]] = OpVariable [[Ptr_PSInput]] Function
// CHECK: [[acInput2:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[input]] %int_2 %uint_0
// CHECK: [[loadInput2:%[0-9]+]] = OpLoad %v3float [[acInput2]]
// CHECK: [[storeRet:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[retVal]] %int_2 %uint_0
// CHECK: OpStore [[storeRet]] [[loadInput2]]