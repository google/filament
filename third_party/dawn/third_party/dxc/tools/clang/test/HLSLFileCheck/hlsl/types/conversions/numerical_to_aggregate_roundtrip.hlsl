// RUN: %dxc -T vs_6_0 -E main %s | FileCheck %s

// Test round-trip conversions from scalar/vector/matrices to structs/arrays and back
// If the round-trip conversion succeeds, we assume both single-way conversions did too.
// Does not test numerical conversions.

// Whenever possible, use 4 members so we can convert between all structs, arrays, int4 and int2x2
struct s_int { int x; }; // For scalar tests
struct s_three_ints { int x, y, z; }; // For truncation tests
struct s_ints { int x, y, z, w; };
struct s_vecs { int2 xy, zw; };
struct s_mat { int2x2 mat; };
struct s_mat_3x3 { int3x3 mat; }; // For truncation tests
struct s_structs
{
    struct { int x, y; } xy;
    struct { int x, y; } zw;
};
struct s_arrays { int xy[2]; int zw[2]; };
struct s_empty_structs { struct {} _pre; int x, y; struct {} _mid; int z, w; struct {} _post; };

typedef int a_int[1];
typedef int a_three_ints[3];
typedef int a_ints[4];
typedef int2 a_vecs[2];
typedef int2x2 a_mat[1];
typedef int3x3 a_mat_3x3[1];
typedef struct { int x, y; } a_structs[2];
typedef int a_ints_2d[2][2];

AppendStructuredBuffer<int4> buffer;

void output_i(int value) { buffer.Append(int4(value, 0, 0, 0)); }
void output_v1(int1 value) { buffer.Append(int4(value.x, 0, 0, 0)); }
void output_v2(int2 value) { buffer.Append(int4(value.x, value.y, 0, 0)); }
void output_v4(int4 value) { buffer.Append(value); }
void output_m1x1(int1x1 value) { buffer.Append(int4(value._11, 0, 0, 0)); }
void output_m2x2(int2x2 value) { buffer.Append(int4(value._11, value._12, value._21, value._22)); }

void output_separator() { buffer.Append((int4)8888); }

void main() {
    int4 v4 = int4(1, 2, 3, 4);
    int2x2 m2x2 = int4(11, 12, 21, 22);
    int4x4 m4x4 = int4x4(11, 12, 13, 14, 21, 22, 23, 24, 31, 32, 33, 34, 41, 42, 43, 44); // For truncation tests

    // Scalar cases
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 0, i32 0, i32 0, i8 15)
    // CHECK: 8888
    output_i((int)(s_int)1);
    output_v1((int1)(s_int)int1(1));
    output_m1x1((int1x1)(s_int)int1x1(1));
    output_i((int)(a_int)1);
    output_v1((int1)(a_int)int1(1));
    output_m1x1((int1x1)(a_int)int1x1(1));
    output_separator();

    // 1-to-1 vector/matrix cases
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_ints)v4);
    output_m2x2((int2x2)(s_ints)m2x2);
    output_v4((int4)(a_ints)v4);
    output_m2x2((int2x2)(a_ints)m2x2);
    output_separator();
    
    // With numerical conversions
    
    // With vectors in compound type
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_vecs)v4);
    output_m2x2((int2x2)(s_vecs)m2x2);
    output_v4((int4)(a_vecs)v4);
    output_m2x2((int2x2)(a_vecs)m2x2);
    output_separator();
    
    // With matrices in compound type
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_mat)v4);
    output_m2x2((int2x2)(s_mat)m2x2);
    output_v4((int4)(a_mat)v4);
    output_m2x2((int2x2)(a_mat)m2x2);
    output_separator();
    
    // With homogeneous nesting (struct of structs, array of arrays)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_structs)v4);
    output_m2x2((int2x2)(s_structs)m2x2);
    output_v4((int4)(a_ints_2d)v4);
    output_m2x2((int2x2)(a_ints_2d)m2x2);
    output_separator();
    
    // With heterogeneous nesting (struct of arrays, array of structs)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_arrays)v4);
    output_m2x2((int2x2)(s_arrays)m2x2);
    output_v4((int4)(a_structs)v4);
    output_m2x2((int2x2)(a_structs)m2x2);
    output_separator();

    // With nested empty struct
    // CHECK: i32 1, i32 2, i32 3, i32 4, i8 15)
    // CHECK: i32 11, i32 12, i32 21, i32 22, i8 15)
    // CHECK: 8888
    output_v4((int4)(s_empty_structs)v4);
    output_m2x2((int2x2)(s_empty_structs)m2x2);
    output_separator();

    // Truncation case
    // Casting a 2D matrix to a smaller struct or struct to smaller 2D matrix is illegal
    // CHECK: i32 1, i32 2, i32 0, i32 0, i8 15)
    // CHECK: i32 1, i32 2, i32 0, i32 0, i8 15)
    // CHECK: 8888
    output_v2((int2)(s_three_ints)v4);
    output_v2((int2)(a_three_ints)v4);
    output_separator();
}