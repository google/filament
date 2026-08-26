// REQUIRES: dxil-1-10

// RUN: %dxc -T cs_6_10 -E main -fcgl %s | FileCheck %s --check-prefix=FCGL
// RUN: %dxc -T cs_6_10 -E main %s | FileCheck %s

// FCGL: call i32 @"dx.hl.op.rn.i32 (i32)"(i32 392)
// FCGL: call i32 @"dx.hl.op.rn.i32 (i32)"(i32 391)

// CHECK: %[[Index:[^ ]+]] = call i32 @dx.op.getGroupWaveIndex(i32 -2147483647)  ; GetGroupWaveIndex()
// CHECK: %[[Count:[^ ]+]] = call i32 @dx.op.getGroupWaveCount(i32 -2147483646)  ; GetGroupWaveCount()
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{[^,]+}}, i32 0, i32 0, i32 %[[Index]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)
// CHECK: call void @dx.op.rawBufferStore.i32(i32 140, %dx.types.Handle %{{[^,]+}}, i32 16, i32 0, i32 %[[Count]], i32 undef, i32 undef, i32 undef, i8 1, i32 4)

RWStructuredBuffer<uint> output0 : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();

    output0[0] = waveIdx;
    output0[16] = waveCount;
}
