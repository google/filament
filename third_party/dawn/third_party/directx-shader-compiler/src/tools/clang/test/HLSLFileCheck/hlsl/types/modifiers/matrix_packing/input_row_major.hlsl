// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test reading input matrix elements in both orientations.

// CHECK: call i32 @dx.op.loadInput.i32(i32 4, i32 0, i32 1, i8 2, i32 undef)
int main(row_major int4x4 r : R) : OUT { return r._23; }