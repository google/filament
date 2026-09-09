// RUN: %dxc -E main -T vs_6_0 -no-warnings %s | FileCheck %s

// Tests that pass-by-value/copy-in semantics are respected
// with aggregate types (arrays and structs).

struct S { int f; };
void set_elem(int a[1]) { a[0] = 42; }
void set_field(S s) { s.f = 42; }
int2 main() : OUT
{
    int a[1] = { 1 };
    set_elem(a);
    S s = { 1 };
    set_field(s);

    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 0, i32 1)
    // CHECK: call void @dx.op.storeOutput.i32(i32 5, i32 0, i32 0, i8 1, i32 1)
    return int2(a[0], s.f);
}