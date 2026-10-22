// RUN: %dxc -E main -T vs_6_2 /Zpr %s | FileCheck %s

// Tests that entry point functions can return arrays of vectors.

typedef int2x2 A[2];
A main() : OUT
{
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 11)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 12)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 21)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 1, i32 22)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 2, i8 0, i32 111)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 2, i8 1, i32 112)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 3, i8 0, i32 121)
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 3, i8 1, i32 122)
  A a = { int2x2(11, 12, 21, 22), int2x2(111, 112, 121, 122) };
  return a;
}