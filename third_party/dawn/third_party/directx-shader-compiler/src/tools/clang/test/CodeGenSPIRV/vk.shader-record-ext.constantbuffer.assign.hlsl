// RUN: %dxc -T lib_6_7 -fspv-target-env=vulkan1.1spirv1.4 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability RayTracingKHR
// CHECK: OpExtension "SPV_KHR_ray_query"
// CHECK: OpExtension "SPV_KHR_ray_tracing"

struct Foo
{
    float m_x;
};

// CHECK: %g_pc = OpVariable %_ptr_ShaderRecordBufferKHR_type_ConstantBuffer_Foo ShaderRecordBufferKHR
[[vk::shader_record_ext]] ConstantBuffer<Foo> g_pc;
RWStructuredBuffer<float> g_buff;

float mul1(Foo m, float4 v)
{
	return m.m_x + v.x;
}

[shader("raygeneration")] void main()
{
    // CHECK: OpLoad %type_ConstantBuffer_Foo %g_pc
    g_buff[0] = mul1(g_pc, float4(1, 0, 0, 1));
}
