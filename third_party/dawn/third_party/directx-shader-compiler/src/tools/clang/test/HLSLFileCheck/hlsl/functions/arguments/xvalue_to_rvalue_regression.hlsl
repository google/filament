// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test for a crash because some intrinsics like StructuredBuffer::Load()
// used to return rvalue references, which classified as an xvalue by clang
// and misbehaved when used as an argument to a function expecting a prvalue.

// CHECK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32
// CHECK: extractvalue %dx.types.ResRet.i32
// CHECK: call void @dx.op.bufferStore.i32

struct S { int x; };
StructuredBuffer<S> structbuf;
AppendStructuredBuffer<S> appbuf;
void main() { appbuf.Append(structbuf.Load(0)); }