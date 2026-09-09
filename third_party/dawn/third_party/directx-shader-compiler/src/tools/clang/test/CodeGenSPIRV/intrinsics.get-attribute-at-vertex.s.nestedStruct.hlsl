// RUN: %dxc -T ps_6_1 -E main -spirv -Od %s 2>&1 | FileCheck %s

enum VertexID { 
    FIRST = 0,
    SECOND = 1,
    THIRD = 2
};
struct PSInput1
{
    float3 color3: COLOR3;
};

struct PSInput
{
    float4 position : SV_POSITION;
    nointerpolation float3 color1 : COLOR1;
    nointerpolation PSInput1 pi : COLOR2;
};

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate [[color1:%[a-zA-Z0-9_]+]] Location 0
// CHECK: OpDecorate [[color2:%[a-zA-Z0-9_]+]] Location 1
// CHECK: OpDecorate [[color1]] PerVertexKHR
// CHECK: OpDecorate [[color2]] PerVertexKHR
// CHECK: [[PSInput1:%[a-zA-Z0-9_]+]] = OpTypeStruct %_arr_v3float_uint_3
// CHECK: [[PSInput:%[a-zA-Z0-9_]+]] = OpTypeStruct %v4float %_arr_v3float_uint_3 [[PSInput1]]
// CHECK: [[color1]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[color2]] = OpVariable %_ptr_Input__arr_v3float_uint_3 Input
// CHECK: [[loadc1:%[0-9]+]] = OpLoad %_arr_v3float_uint_3 [[color1]]
// CHECK: [[loadc2:%[0-9]+]] = OpLoad %_arr_v3float_uint_3 [[color2]]
// CHECK: [[loadpi1:%[0-9]+]] = OpCompositeConstruct [[PSInput1]] [[loadc2]]
// CHECK: [[loadpi:%[0-9]+]] = OpCompositeConstruct [[PSInput]] [[frag:%[0-9]+]] [[loadc1]] [[loadpi1]]
float3 main( PSInput input ) : SV_Target
{
    float3 vColor0 = input.color1;
    float3 vColor1 = GetAttributeAtVertex( input.color1, VertexID::SECOND );
    float3 vColor2 = input.pi.color3;
    float3 vColor3 = GetAttributeAtVertex( input.pi.color3, VertexID::THIRD );
    return (vColor0 + vColor1 + vColor2 + vColor3);
}

// CHECK: [[input:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function_PSInput
// CHECK: %vColor2 = OpVariable %_ptr_Function_v3float Function
// CHECK: [[acColor3:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float [[input]] %int_2 %int_0 %uint_0
// CHECK: [[loadColor3:%[0-9]+]] = OpLoad %v3float [[acColor3]]
// CHECK: OpStore %vColor2 [[loadColor3]]