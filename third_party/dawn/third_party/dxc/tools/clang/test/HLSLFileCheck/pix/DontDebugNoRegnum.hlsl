// RUN: %dxc -Tlib_6_6 %s | %opt -S -dxil-annotate-with-virtual-regs -hlsl-dxil-debug-instrumentation | %FileCheck %s

// Check that the instrumentation does NOT instrument an instruction that has no dxil-inst-num metadata
// The load instruction should not be instrumented. If it is, we can expect an "atomicBinOp", emitted
// by the instrumentation, to be generated before the handle value is used, so assert that there
// is no such atomicBinOp:

// CHECK: [[HandleNum:%[0-9]+]] = load %dx.types.Handle,
// CHECK-NOT: call i32 @dx.op.atomicBinOp.i32(i32 78
// CHECK: @dx.op.createHandleForLib.dx.types.Handle(i32 160, %dx.types.Handle [[HandleNum]]

RWByteAddressBuffer buffer : register(u0);

[shader("raygeneration")] 
void main() {
  buffer.Store(0, 42);
}