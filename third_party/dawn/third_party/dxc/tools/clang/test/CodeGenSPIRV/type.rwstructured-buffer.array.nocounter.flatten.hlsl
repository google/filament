// RUN: %dxc -T ps_6_6 -E main -fspv-flatten-resource-arrays -O0 %s -spirv | FileCheck %s

struct PSInput
{
	uint idx : COLOR;
};

// CHECK: OpDecorate %g_rwbuffer_0_ DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer_0_ Binding 0
// CHECK: OpDecorate %g_rwbuffer_1_ DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer_1_ Binding 1
// CHECK: OpDecorate %g_rwbuffer_2_ DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer_2_ Binding 2
// CHECK: OpDecorate %g_rwbuffer_3_ DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer_3_ Binding 3
// CHECK: OpDecorate %g_rwbuffer_4_ DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer_4_ Binding 4

// CHECK: %g_rwbuffer_0_ = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
// CHECK: %g_rwbuffer_1_ = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
// CHECK: %g_rwbuffer_2_ = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
// CHECK: %g_rwbuffer_3_ = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
// CHECK: %g_rwbuffer_4_ = OpVariable %_ptr_Uniform_type_RWStructuredBuffer_uint Uniform
RWStructuredBuffer<uint> g_rwbuffer[5] : register(u0, space2);

float4 main(PSInput input) : SV_TARGET
{
	return g_rwbuffer[input.idx][0];
}
