// RUN: %dxc /T vs_6_0 /E main %s | FileCheck %s

// Test that matrix packing order does not cause undesirable transposes
// with constant initialization lists. Constant initializers should
// be emitted in the target memory packing order and "fixed up"
// when the static variable gets loaded.

AppendStructuredBuffer<int4> buf;

void main()
{
    // Test building matrix constants
    // Matrices need to be hidden in structures because
    // otherwise we do not consider them for constant initialization.
    static const struct { row_major int2x2 mat; } r = { 11, 12, 21, 22 };
    static const struct { column_major int2x2 mat; } c = { 11, 12, 21, 22 };
    
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    buf.Append((int4)r.mat);
    buf.Append((int4)c.mat);

    // Convert between packing orders (ie test flattening matrix constants).
    // Use two fields per variable so that constant init list logic is used.
    // If there is a single initializer, it becomes a mere cast. 
    static const struct { row_major int2x2 mat1, mat2; } r2 = { r, c };
    static const struct { column_major int2x2 mat1, mat2; } c2 = { r, c };

    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    buf.Append((int4)r2.mat1);
    buf.Append((int4)r2.mat2);
    buf.Append((int4)c2.mat1);
    buf.Append((int4)c2.mat2);
}