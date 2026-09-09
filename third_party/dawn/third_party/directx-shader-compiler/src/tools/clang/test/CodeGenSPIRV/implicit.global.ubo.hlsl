// RUN: %dxc -T ps_6_0 -E main %s -spirv | FileCheck %s

// This test checks that the specialization constant and push constants are not
// included in the implicit global ubo.

// CHECK: OpDecorate %mySpecializationConstant SpecId 1
// CHECK: %mySpecializationConstant = OpSpecConstant %int 0
[[ vk::constant_id ( 1 )]] const int mySpecializationConstant = 0;

// CHECK:  %push_const = OpVariable %_ptr_PushConstant_type_PushConstant_ PushConstant
[[ vk::push_constant]] struct { int value; } push_const;



float myGlobal;

float4 main(float4 col : COLOR) : SV_Target0
{
	if(mySpecializationConstant && push_const.value)
		return 0.xxxx;
	
	return myGlobal.xxxx;
}
