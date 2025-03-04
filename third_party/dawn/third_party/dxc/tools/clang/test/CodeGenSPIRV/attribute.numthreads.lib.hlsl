// RUN: %dxc -T lib_6_4 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExecutionMode %entryHistogram LocalSize 16 16 1
[shader("compute")]
[numthreads(16, 16, 1)]
void entryHistogram(uint3 id: SV_DispatchThreadID, uint idx: SV_GroupIndex)
{
}

// CHECK: OpExecutionMode %entryAverage LocalSize 256 1 1
[shader("compute")]
[numthreads(256, 1, 1)]
void entryAverage(uint3 id: SV_DispatchThreadID, uint idx: SV_GroupIndex)
{
}

