// RUN: %dxc -E main -T as_6_5 %s | FileCheck %s
// RUN: %dxc -E main -T as_6_5 %s | %D3DReflect %s | FileCheck -check-prefix=REFL %s

// CHECK: Amplification Shader
// CHECK: NumThreads=(32,1,1)
// CHECK: dx.op.dispatchMesh.struct.Payload

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

// REFL: TempArrayCount: 0
// REFL: DynamicFlowControlCount: 0
// REFL: ArrayInstructionCount: 0
// REFL: TextureNormalInstructions: 0
// REFL: TextureLoadInstructions: 0
// REFL: TextureCompInstructions: 0
// REFL: TextureBiasInstructions: 0
// REFL: TextureGradientInstructions: 0
// REFL: FloatInstructionCount: 0
// REFL: IntInstructionCount: 0
// REFL: UintInstructionCount: 0
// REFL: CutInstructionCount: 0
// REFL: EmitInstructionCount: 0
// REFL: cBarrierInstructions: 0
// REFL: cInterlockedInstructions: 0
// REFL: cTextureStoreInstructions: 0
