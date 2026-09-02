// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct Foo
{
    float m_x;
};

// CHECK: %g_pc = OpVariable %_ptr_PushConstant_type_ConstantBuffer_Foo PushConstant
[[vk::push_constant]] ConstantBuffer<Foo> g_pc;
RWStructuredBuffer<float> g_buff;

float mul1(Foo m, float4 v)
{
	return m.m_x + v.x;
}

[numthreads(1, 1, 1)] void main()
{
    // CHECK: OpLoad %type_ConstantBuffer_Foo %g_pc
    g_buff[0] = mul1(g_pc, float4(1, 0, 0, 1));
}
