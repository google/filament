// RUN: not %dxc -T cs_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

static uint g_globalMeshCounter;

[numthreads(1, 1, 1)]
void main(
	uint groupIdx : SV_GroupID,
	uint localIdx : SV_GroupThreadID)
{
	uint offset;
	InterlockedAdd(g_globalMeshCounter, 1, offset);
}

// CHECK: :11:17: error: using static variable or function scope variable in interlocked operation is not allowed
