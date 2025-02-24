// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Memory scope : Workgroup = 0x2 = 2
// Semantics: WorkgroupMemory | AcquireRelease = 0x100 | 0x8 = 264

[numthreads(1,1,1)]
void main() {
// CHECK: OpMemoryBarrier %uint_2 %uint_264
  GroupMemoryBarrier();
}
