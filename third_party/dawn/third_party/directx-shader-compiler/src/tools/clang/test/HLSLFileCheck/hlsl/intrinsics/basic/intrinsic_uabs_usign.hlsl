// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test the unsigned version of the abs and sign intrinsics

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 -1)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 1)

uint2 main() : OUT
{
    return uint2(abs((uint)0xFFFFFFFF), sign((uint)0xFFFFFFFF));
}