// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Test that intrinsics can be overloaded without
// shadowing the original definition using incorrect type shapes
// and also disallowed content casts

// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 42)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 1)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 2, i32 1)
// CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 3, i32 0)

struct Struct { int x; };
int abs(Struct s) { return 42; }
int asint(bool b) { return int(b); }

int4 main() : OUT
{
    Struct s = { -1 };
    bool b = true;
    return int4(abs(s), // Should call struct overload
        abs(-1), // Should call intrinsic
        asint(b), // Should call bool overload
        asint(0.0));// Should call intrinsic
}