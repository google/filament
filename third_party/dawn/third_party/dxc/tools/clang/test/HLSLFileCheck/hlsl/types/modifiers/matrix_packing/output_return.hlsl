// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test that outputting a matrix value through a return statement
// correctly takes the return parameter orientation into account.

typedef row_major int2x2 rmi2x2;
typedef column_major int2x2 cmi2x2;
rmi2x2 main() : OUT
{
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 11)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 12)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 0, i32 21)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 1, i8 1, i32 22)
    return cmi2x2(11, 12, 21, 22);
}