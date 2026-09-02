// RUN: %dxc -Emain -Tcs_6_10 %s | %opt -S -dxil-annotate-with-virtual-regs -hlsl-dxil-debugbreak-instrumentation | %FileCheck %s

// Verify the PIX UAV handle is created for DebugBreak instrumentation:
// CHECK: %PixUAVHandle = call %dx.types.Handle @dx.op.createHandleFromBinding(

// Verify an AtomicBinOp (opcode 78) was emitted to record the DebugBreak hit:
// CHECK: %DebugBreakBitSet = call i32 @dx.op.atomicBinOp.i32(i32 78, %dx.types.Handle

// Verify the original DebugBreak call was removed:
// CHECK-NOT: @dx.op.debugBreak

[numthreads(1, 1, 1)]
void main() {
    DebugBreak();
}
