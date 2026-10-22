// REQUIRES: dxil-1-10
// RUN: %dxc -E main -T cs_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK: DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Compute Shader
// CHECK-NEXT:  NumThreads=(64,1,1)
// CHECK-NEXT:  NumBytesGroupSharedMemory: 0
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295

// Test that NumBytesGroupSharedMemory is 0 when there is no groupshared memory.

RWBuffer<uint> output : register(u0);

[numthreads(64, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
  output[tid.x] = tid.x * 2;
}
