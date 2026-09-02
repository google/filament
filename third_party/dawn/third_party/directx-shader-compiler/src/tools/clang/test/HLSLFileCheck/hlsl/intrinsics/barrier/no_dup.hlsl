// RUN: %dxc -T cs_6_3 %s | FileCheck %s

// Make sure only 2 barrier.
// CHECK: call void @dx.op.barrier(i32 80, i32 9)
// CHECK: call void @dx.op.barrier(i32 80, i32 9)
// CHECK-NOT: call void @dx.op.barrier(i32 80, i32 9)

RWStructuredBuffer<uint> _CoarseStencilBuffer;
uint stencilBufferWidth;

groupshared uint g_GroupsharedValue;

[numthreads(8, 8, 1)]
void main(uint3 groupId : SV_GroupID,
          uint threadIndex : SV_GroupIndex)
{
	uint threadValue = 1 << (threadIndex/2);

    bool isFirstThreadInGroup = threadIndex == 0;

    if (isFirstThreadInGroup)
        g_GroupsharedValue = 0;

    GroupMemoryBarrierWithGroupSync();
    InterlockedOr(g_GroupsharedValue, threadValue);
    GroupMemoryBarrierWithGroupSync();

    if (isFirstThreadInGroup)
    {
        uint addressIndex = groupId.y * stencilBufferWidth + groupId.x;
        _CoarseStencilBuffer[addressIndex] = g_GroupsharedValue;
    }
}