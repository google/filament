// RUN: not %dxc -T lib_6_4 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

[shader("compute")]
[numthreads(16, 16, 1)]
void entryHistogram(uint3 id: SV_DispatchThreadID, uint idx: SV_GroupIndex)
{
}

// CHECK: 11:6: error: compute entry point must have a valid numthreads attribute
[shader("compute")]
void entryAverage(uint3 id: SV_DispatchThreadID, uint idx: SV_GroupIndex)
{
}
