// RUN: %dxc -Emain -Tcs_6_10 %s | %opt -S -dxil-annotate-with-virtual-regs -hlsl-dxil-debugbreak-instrumentation | %FileCheck %s

// Verify the PIX UAV handle is created:
// CHECK: %PixUAVHandle = call %dx.types.Handle @dx.op.createHandleFromBinding(

// Verify two AtomicBinOp calls were emitted (one per DebugBreak):
// CHECK: DebugBreakBitSet{{.*}} = call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle
// CHECK: DebugBreakBitSet{{.*}} = call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle

// Verify no DebugBreak calls remain:
// CHECK-NOT: @dx.op.debugBreak

RWByteAddressBuffer buf : register(u0);

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID) {
    if (tid.x == 0)
        DebugBreak();

    buf.Store(0, tid.x);

    if (tid.x == 1)
        DebugBreak();
}
