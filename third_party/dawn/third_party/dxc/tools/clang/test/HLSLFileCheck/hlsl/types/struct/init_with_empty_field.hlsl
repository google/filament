// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Regression test for GitHub #1873, which caused an error
// when initializing a struct containing an empty struct field.

int2 main() : OUT
{
    struct
    {
        struct {} _1;
        int x;
        struct {} _2;
        int y;
        struct {} _3;
    } s = { 1, 2 };

    // CHECK: @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
    // CHECK: @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 2)
    return int2(s.x, s.y);
}