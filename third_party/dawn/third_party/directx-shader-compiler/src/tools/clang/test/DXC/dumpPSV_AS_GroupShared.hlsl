// REQUIRES: dxil-1-10
// RUN: %dxc -E main -T as_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Amplification Shader
// CHECK-NEXT:  NumThreads=(32,1,1)
// CHECK-NEXT:  NumBytesGroupSharedMemory: 128
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295

// Test that NumBytesGroupSharedMemory is calculated correctly for AS with groupshared.
// groupshared float4 sharedData[8] = 8 * 16 bytes = 128 bytes

struct Payload {
    float4 data[4];
};

groupshared float4 sharedData[8];

[numthreads(32, 1, 1)]
void main(uint gtid : SV_GroupIndex) {
    sharedData[gtid % 8] = float4(gtid, 0, 0, 0);
    GroupMemoryBarrierWithGroupSync();
    
    Payload pld;
    pld.data[0] = sharedData[0];
    pld.data[1] = sharedData[1];
    pld.data[2] = sharedData[2];
    pld.data[3] = sharedData[3];
    DispatchMesh(1, 1, 1, pld);
}
