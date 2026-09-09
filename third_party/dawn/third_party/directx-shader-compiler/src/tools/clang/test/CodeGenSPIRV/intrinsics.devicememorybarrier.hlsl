// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// Memory scope : Device = 0x1 = 1
// Semantics: ImageMemory | UniformMemory | AcquireRelease = 0x800 | 0x40 | 0x8 = 2120

[numthreads(1,1,1)]
void main() {
// CHECK: OpMemoryBarrier %uint_1 %uint_2120
  DeviceMemoryBarrier();
}
