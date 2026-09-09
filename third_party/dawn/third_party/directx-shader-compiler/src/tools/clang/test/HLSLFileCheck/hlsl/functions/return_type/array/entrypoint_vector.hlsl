// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Tests that entry point functions can return arrays of vectors.

typedef int2 A[2];
A main() : OUT
{
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 2)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 3)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 1, i32 4)
  A a = { int2(1, 2), int2(3, 4) };
  return a;
}