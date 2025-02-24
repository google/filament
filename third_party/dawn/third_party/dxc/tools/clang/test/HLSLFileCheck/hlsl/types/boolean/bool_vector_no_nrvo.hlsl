// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test for a crash where bool vectors where eligible
// for named return value optimizations, which caused register/memory
// representation mismatches.

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 0)

bool3 main() : OUT {
  bool3 b = false;
  return b;
}
