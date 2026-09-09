// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test that implicitly truncating a matrix keeps
// the same matrix elements no matter the orientation of the matrices.

void main(out column_major int1x2 result : OUT)
{
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 11)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 12)
    column_major int2x2 value = int2x2(11, 12, 21, 22);
    result = value;
}