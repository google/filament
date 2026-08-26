// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Execution scope : Workgroup = 0x2 = 2
// Memory scope : Workgroup = 0x2 = 2
// Semantics: WorkgroupMemory | AcquireRelease = 0x100 | 0x8 = 264

[numthreads(1,1,1)]
void main() {
// CHECK: OpControlBarrier %uint_2 %uint_2 %uint_264
  GroupMemoryBarrierWithGroupSync();
}
