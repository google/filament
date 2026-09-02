// RUN: %dxc -E main -T vs_6_2 %s | FileCheck %s

// Tests that entry point functions can return arrays,
// and that elements are considered to be on different rows.

typedef int A[2];
A main() : OUT
{
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 2)
  A a = { 1, 2 };
  return a;
}