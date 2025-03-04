// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s | XFail GitHub #1887

// Test that global functions can shadow intrinsics.

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 42)

int abs(int x) { return 42; }
int main() : OUT { return abs(-1); }