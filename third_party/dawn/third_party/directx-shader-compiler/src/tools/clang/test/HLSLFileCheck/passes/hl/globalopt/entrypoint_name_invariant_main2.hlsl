// RUN: %dxc -T vs_6_0 -E main2 %s | FileCheck %s

// Regression test for #2318, where the globalopt had behavior
// conditional on the name of the main function.

static uint g[1] = { 0 };
uint main2() : OUT
{
  // CHECK-NOT: load i32
  // CHECK-NOT: store i32
  g[0]++;
  // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
  return g[0];
}