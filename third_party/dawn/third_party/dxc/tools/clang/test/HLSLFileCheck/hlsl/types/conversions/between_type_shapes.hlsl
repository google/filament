// RUN: %dxc -E main -T vs_6_0 -no-warnings -HV 2018 %s | FileCheck -check-prefix=DXC %s

// Tests all implicit conversions and explicit casts between type shapes
// (scalars, vectors, matrices, arrays and structs).
// A matching syntactic test confirms which conversions/casts are valid and which are not.
// The codegen for all valid conversion/casts is exercised here.

// We test using scalars, vectors, matrices, arrays and structs in sizes 1, 2 and 4.
// Size 1 are for direct conversions to/from scalars
// Size 2 is for most other conversions (avoiding potential special cases for single elements)
// Size 4 is for conversions to/from matrices (avoiding potential special cases for single rows/columns)
// As an exception, we use int3x3 to cast int2x2 up (avoiding potential special cases for single rows/columns)

typedef int A1[1];
typedef int A2[2];
typedef int A4[4];
typedef int A5[5];
struct S1 { int a; };
struct S2 { int a, b; };
struct S4 { int a, b, c, d; };
struct S5 { int a, b, c, d, e; };

AppendStructuredBuffer<int4> buffer;
// Avoid overloading since it plays into conversions
// _i means scalar int, to avoid confusion with _s for structs
void output_i(int i) { buffer.Append(int4(i, 0, 0, 0)); }
void output_v1(int1 v) { buffer.Append(int4(v.x, 0, 0, 0)); }
void output_v2(int2 v) { buffer.Append(int4(v.x, v.y, 0, 0)); }
void output_v4(int4 v) { buffer.Append(v); }
void output_m1x1(int1x1 m) { buffer.Append(int4(m._11, 0, 0, 0)); }
void output_m1x2(int1x2 m) { buffer.Append(int4(m._11, m._12, 0, 0)); }
void output_m2x1(int2x1 m) { buffer.Append(int4(m._11, m._21, 0, 0)); }
void output_m2x2(int2x2 m) { buffer.Append(int4(m._11, m._12, m._21, m._22)); }
void output_m3x3(int3x3 m)
{
    buffer.Append(int4(m._11, m._12, m._13, 0));
    buffer.Append(int4(m._21, m._22, m._23, 0));
    buffer.Append(int4(m._31, m._32, m._33, 0));
}
void output_a1(A1 a) { buffer.Append(int4(a[0], 0, 0, 0)); }
void output_a2(A2 a) { buffer.Append(int4(a[0], a[1], 0, 0)); }
void output_a4(A4 a) { buffer.Append(int4(a[0], a[1], a[2], a[3])); }
void output_s1(S1 s) { buffer.Append(int4(s.a, 0, 0, 0)); }
void output_s2(S2 s) { buffer.Append(int4(s.a, s.b, 0, 0)); }
void output_s4(S4 s) { buffer.Append(int4(s.a, s.b, s.c, s.d)); }

// This is only to make it easier to match the output to the code.
void output_separator() { buffer.Append(int4(8888, 8888, 8888, 8888)); }

void main()
{
    int i = 1;
    int1 v1 = int1(1);
    int2 v2 = int2(1, 2);
    int4 v4 = int4(1, 2, 3, 4);
    int1x1 m1x1 = int1x1(11);
    int1x2 m1x2 = int1x2(11, 12);
    int2x1 m2x1 = int2x1(11, 21);
    int2x2 m2x2 = int2x2(11, 12, 21, 22);
    int1x3 m1x3 = int1x3(11, 12, 13);
    int2x3 m2x3 = int2x3(11, 12, 13, 21, 22, 23);
    int3x1 m3x1 = int3x1(11, 21, 31);
    int3x2 m3x2 = int3x2(11, 12, 21, 22, 31, 32);
    int3x3 m3x3 = int3x3(11, 12, 13, 21, 22, 23, 31, 32, 33);
    A1 a1 = { 1 };
    A2 a2 = { 1, 2 };
    A4 a4 = { 1, 2, 3, 4 };
    A5 a5 = { 1, 2, 3, 4, 5 };
    S1 s1 = { 1 };
    S2 s2 = { 1, 2 };
    S4 s4 = { 1, 2, 3, 4 };
    S5 s5 = { 1, 2, 3, 4, 5 };

    // =========== Scalar/single-element ===========
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i(v1);
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_i(m1x1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i((int)a1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i((int)s1);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1(i);
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_v1(m1x1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1((int1)a1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1((int1)s1);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1(i);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1(v1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1((int1x1)a1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1((int1x1)s1);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1((A1)i);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1((A1)v1);
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_a1((A1)m1x1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1(s1); 
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1((A1)s1);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_s1((S1)i);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_s1((S1)v1);
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_s1((S1)m1x1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_s1(a1);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_s1((S1)a1);

    // DXC: 8888
    output_separator();

    // =========== Truncation to scalar/single-element ===========
    // Single element sources already tested
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i(v2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_i(m2x2); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i((int)a2);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_i((int)s2);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1(v2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_v1(m2x2); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1((int1)a2);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_v1((int1)s2);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1(v2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(11,0,0,0)
    output_m1x1(m2x2); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1((int1x1)a2);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_m1x1((int1x1)s2);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1((A1)a2);
    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_a1((A1)s2);

    // DXC: i32 1, i32 0, i32 0, i32 0, i8 15)
    // FXC: l(1,0,0,0)
    output_s1((S1)a2);
    // DXC crashes (GitHub #1970)
    // FXC: l(1,0,0,0)
    // output_s1((S1)s2);
    
    // DXC: 8888
    output_separator();

    // =========== Splatting ===========
    // Single element dests already tested
    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_v2(i);
    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_v2(v1);
    // DXC: i32 11, i32 11, i32 0, i32 0, i8 15)
    // FXC: l(11,11,0,0)
    output_v2(m1x1);

    // DXC: i32 1, i32 1, i32 1, i32 1, i8 15)
    // FXC: l(1,1,1,1)
    output_m2x2(i);
    // DXC: i32 1, i32 1, i32 1, i32 1, i8 15)
    // FXC: l(1,1,1,1)
    output_m2x2(v1);
    // DXC: i32 11, i32 11, i32 11, i32 11, i8 15)
    // FXC: l(11,11,11,11)
    output_m2x2(m1x1);

    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_a2((A2)i);
    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_a2((A2)v1);
    // DXC: i32 11, i32 11, i32 0, i32 0, i8 15)
    // FXC: l(11,11,0,0)
    output_a2((A2)m1x1);

    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_s2((S2)i);
    // DXC: i32 1, i32 1, i32 0, i32 0, i8 15)
    // FXC: l(1,1,0,0)
    output_s2((S2)v1);
    // DXC: i32 11, i32 11, i32 0, i32 0, i8 15)
    // FXC: l(11,11,0,0)
    output_s2((S2)m1x1);
    
    // DXC: 8888
    output_separator();

    // =========== Element-preserving ===========
    // Single element sources/dests already tested
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_v2(m1x2);
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_v2(m2x1);
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_v4(m2x2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_v2((int2)a2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_v2((int2)s2);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2(v2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m2x1(v2);
    // DXC: i32 1, i32 2, i32 3, i32 4, i8 15)
    // FXC: l(1,2,3,4)
    output_m2x2(v4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2((int1x2)a2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m2x1((int2x1)a2);
    // DXC: i32 1, i32 2, i32 3, i32 4, i8 15)
    // FXC: l(1,2,3,4)
    output_m2x2((int2x2)a4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2((int1x2)s2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m2x1((int2x1)s2);
    // DXC: i32 1, i32 2, i32 3, i32 4, i8 15)
    // FXC: l(1,2,3,4)
    output_m2x2((int2x2)s4);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2((A2)v2);
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_a2((A2)m1x2);
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_a2((A2)m2x1);
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_a4((A4)m2x2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2(s2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2((A2)s2);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_s2((S2)v2);
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_s2((S2)m1x2);
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_s2((S2)m2x1);
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_s4((S4)m2x2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_s2(a2);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_s2((S2)a2);
    
    // DXC: 8888
    output_separator();

    // =========== Truncating ===========
    // Single element dests already tested
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_v2(v4); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_v2(m1x3); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_v2(m3x1); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_v2((int2)a4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_v2((int2)s4);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2(v4); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC fails with internal error: invalid sequence/cast expression
    output_m2x1(v4); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_m1x2(m1x3); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_m1x2(m2x2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_m2x1(m3x1); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_m2x1(m2x2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_m2x2(m2x3); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_m2x2(m3x2); // warning: implicit truncation of vector type
    // DXC: i32 11, i32 12, i32 21, i32 22, i8 15)
    // FXC: l(11,12,21,22)
    output_m2x2(m3x3); // warning: implicit truncation of vector type
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2((int1x2)a4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC fails with internal error: invalid sequence/cast expression
    output_m2x1((int2x1)a4);
    // DXC: i32 1, i32 2, i32 3, i32 4, i8 15)
    // FXC rejects with error X3017: cannot convert from 'typedef int[5]' to 'int2x2'
    output_m2x2((int2x2)a5);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_m1x2((int1x2)s4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC fails with internal error: invalid sequence/cast expression
    output_m2x1((int2x1)s4);
    // DXC: i32 1, i32 2, i32 3, i32 4, i8 15)
    // FXC rejects with error X3017: cannot convert from 'struct S5' to 'int2x2'
    output_m2x2((int2x2)s5);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2((A2)v4);
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_a2((A2)m1x3);
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_a2((A2)m3x1);
    // DXC accepts (GitHub #1865)
    // FXC rejects with error X3017: cannot convert from 'int2x2' to 'typedef int[2]'
    // output_a2((A2)m2x2);
    // DXC accepts (GitHub #1865)
    // FXC rejects with error X3017: cannot convert from 'int3x3' to 'typedef int[2]'
    // output_a2((A2)m3x3);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2((A2)a4);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_a2((A2)s4);

    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_s2((S2)v4);
    // DXC: i32 11, i32 12, i32 0, i32 0, i8 15)
    // FXC: l(11,12,0,0)
    output_s2((S2)m1x3);
    // DXC: i32 11, i32 21, i32 0, i32 0, i8 15)
    // FXC: l(11,21,0,0)
    output_s2((S2)m3x1);
    // DXC accepts (GitHub #1865)
    // FXC rejects with error X3017: cannot convert from 'int2x2' to 'struct S2'
    // output_s2((S2)m2x2);
    // DXC accepts (GitHub #1865)
    // FXC rejects with error X3017: cannot convert from 'int3x3' to 'struct S2'
    // output_s2((S2)m3x3);
    // DXC: i32 1, i32 2, i32 0, i32 0, i8 15)
    // FXC: l(1,2,0,0)
    output_s2((S2)a4);
    // DXC crashes (GitHub #1970)
    // FXC: l(1,2,0,0)
    // output_s2((S2)s4);
}
