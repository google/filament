// RUN: %dxc -T ps_6_6 -E main -O0 %s -spirv | FileCheck %s

struct PSInput
{
	uint idx : COLOR;
};

// CHECK: OpDecorate %g_rwbuffer DescriptorSet 2
// CHECK: OpDecorate %g_rwbuffer Binding 0
// CHECK: OpDecorate %counter_var_g_rwbuffer DescriptorSet 2
// CHECK: OpDecorate %counter_var_g_rwbuffer Binding 1

// CHECK: %counter_var_g_rwbuffer = OpVariable %_ptr_Uniform__arr_type_ACSBuffer_counter_uint_5 Uniform
// CHECK: %g_rwbuffer = OpVariable %_ptr_Uniform__arr_type_RWStructuredBuffer_uint_uint_5 Uniform
RWStructuredBuffer<uint> g_rwbuffer[5] : register(u0, space2);

float4 main(PSInput input) : SV_TARGET
{
    RWStructuredBuffer<uint> l_rwbuffer[5] = g_rwbuffer;

// CHECK: [[ac1:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_ACSBuffer_counter %counter_var_g_rwbuffer %int_0
// CHECK: [[ac2:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[ac1]] %uint_0
// CHECK: OpAtomicIAdd %int [[ac2]] %uint_1 %uint_0 %int_1
    l_rwbuffer[0].IncrementCounter();

// CHECK: [[ac1_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_RWStructuredBuffer_uint %g_rwbuffer {{%[0-9]+}}
// CHECK: [[ac2_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[ac1_0]] %int_0 %uint_0
// CHECK: OpLoad %uint [[ac2_0]]
    return l_rwbuffer[input.idx][0];
}
