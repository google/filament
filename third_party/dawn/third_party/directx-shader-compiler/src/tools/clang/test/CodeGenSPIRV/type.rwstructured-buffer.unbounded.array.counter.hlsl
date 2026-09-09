// RUN: %dxc -T ps_6_6 -E main -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
	uint idx : COLOR;
};

// CHECK: OpDecorate %g_rwbuffer DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer Binding 0
// CHECK: OpDecorate %counter_var_g_rwbuffer DescriptorSet 2
// CHECK: OpDecorate %counter_var_g_rwbuffer Binding 1

// CHECK: %counter_var_g_rwbuffer = OpVariable %_ptr_Uniform__runtimearr_type_ACSBuffer_counter Uniform
// CHECK: %g_rwbuffer = OpVariable %_ptr_Uniform__runtimearr_type_RWStructuredBuffer_uint Uniform
RWStructuredBuffer<uint> g_rwbuffer[] : register(u0, space2);

float4 main(PSInput input) : SV_TARGET
{
// Correctly increment the counter.
// CHECK: [[ac1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_ACSBuffer_counter %counter_var_g_rwbuffer {{%[0-9]+}}
// CHECK: [[ac2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[ac1]] %uint_0
// CHECK: OpAtomicIAdd %int [[ac2]] %uint_1 %uint_0 %int_1
    g_rwbuffer[input.idx].IncrementCounter();

// Correctly access the buffer.
// CHECK: [[ac1_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_type_RWStructuredBuffer_uint %g_rwbuffer {{%[0-9]+}}
// CHECK: [[ac2_0:%[a-zA-Z0-9_]+]] = OpAccessChain %_ptr_Uniform_uint [[ac1_0]] %int_0 %uint_0
// CHECK: OpLoad %uint [[ac2_0]]
    return g_rwbuffer[input.idx][0];
}
