// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};

struct PSInput
{
    float3 color0 : COLOR0;
    nointerpolation float3 color1 : COLOR1;
    nointerpolation float3 color2 : COLOR2;
};

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate [[color0:%[a-zA-Z0-9_]+]] Location 0
// CHECK: OpDecorate [[color1:%[a-zA-Z0-9_]+]] Location 1
// CHECK: OpDecorate [[color2:%[a-zA-Z0-9_]+]] Location 2
// CHECK: OpDecorate [[color1]] PerVertexKHR
// CHECK: OpDecorate [[color2]] PerVertexKHR
// CHECK: [[PSInput:%[a-zA-Z0-9_]+]] = OpTypeStruct %v3float %_arr_v3float_uint_3 %_arr_v3float_uint_3
// CHECK: [[color0]] = OpVariable %_ptr_Input_v3float Input
// CHECK: [[color1]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[color2]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
float3 main(PSInput input) : SV_Target
{
    float3 vColor0 = input.color0;
    float3 vColor1 = input.color1;
    float3 vColor2 = GetAttributeAtVertex(input.color1, VertexID::SECOND );
    float3 vColor3 = input.color1;
    float3 vColor4 = float3(input.color2.y, input.color1.z, vColor2.y);
    return vColor0 + vColor1 + vColor2 + vColor3 + vColor4;
}
// CHECK: [[input:%[a-zA-Z0-9_]+]] = OpFunctionParameter [[Ptr_PSInput:%[a-zA-Z0-9_]+]]
// CHECK: [[acInputColor2:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[input]] %int_2 %uint_0
// CHECK: [[color2Y:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[acInputColor2]] %int_1
// CHECK: [[loadc2y:%[0-9]+]] = OpLoad %float [[color2Y]]
// CHECK: [[result:%[0-9]+]] = OpCompositeConstruct %v3float [[loadc2y]] [[loadc1z:%[0-9]+]] [[vc2y:%[0-9]+]]