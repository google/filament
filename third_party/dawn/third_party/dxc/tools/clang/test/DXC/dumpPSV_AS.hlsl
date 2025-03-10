// REQUIRES: dxil-1-8
// RUN: %dxc -E main -T as_6_8 %s -Fo %t
// RUN: %dxa %t -dumppsv | FileCheck %s

// CHECK:DxilPipelineStateValidation:
// CHECK-NEXT: PSVRuntimeInfo:
// CHECK-NEXT:  Amplification Shader
// CHECK-NEXT:  NumThreads=(32,1,1)
// CHECK-NEXT:  MinimumExpectedWaveLaneCount: 0
// CHECK-NEXT:  MaximumExpectedWaveLaneCount: 4294967295
// CHECK-NEXT:  UsesViewID: false
// CHECK-NEXT:  SigInputElements: 0
// CHECK-NEXT:  SigOutputElements: 0
// CHECK-NEXT:  SigPatchConstOrPrimElements: 0
// CHECK-NEXT:  SigInputVectors: 0
// CHECK-NEXT:  SigOutputVectors[0]: 0
// CHECK-NEXT:  SigOutputVectors[1]: 0
// CHECK-NEXT:  SigOutputVectors[2]: 0
// CHECK-NEXT:  SigOutputVectors[3]: 0
// CHECK-NEXT:  EntryFunctionName: main
// CHECK-NEXT: ResourceCount : 0

#define NUM_THREADS 32

struct Payload {
    float2 dummy;
    float4 pos;
    float color[2];
};

[numthreads(NUM_THREADS, 1, 1)]
void main()
{
    Payload pld;
    pld.dummy = float2(1.0,2.0);
    pld.pos = float4(3.0,4.0,5.0,6.0);
    pld.color[0] = 7.0;
    pld.color[1] = 8.0;
    DispatchMesh(NUM_THREADS, 1, 1, pld);
}
