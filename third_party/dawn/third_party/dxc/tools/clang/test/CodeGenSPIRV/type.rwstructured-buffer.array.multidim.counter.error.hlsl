// RUN: not %dxc -T ps_6_6 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct PSInput
{
	uint idx : COLOR;
};

RWStructuredBuffer<uint> g_rwbuffer[2][3] : register(u0, space2);

float4 main(PSInput input) : SV_TARGET
{
    g_rwbuffer[1][input.idx].IncrementCounter();
    return g_rwbuffer[input.idx][input.idx][0];
}

// CHECK: :8:26: error: Multi-dimensional arrays of RW/append/consume structured buffers are unsupported in Vulkan
// CHECK: RWStructuredBuffer<uint> g_rwbuffer[2][3] : register(u0, space2);
