// RUN: %dxc -E main -T vs_6_0 -HV 2018 %s | FileCheck %s

// Tests the implementation of unary and binary matrix operators

AppendStructuredBuffer<int1> output_i;
AppendStructuredBuffer<uint1> output_u;
AppendStructuredBuffer<float1> output_f;
AppendStructuredBuffer<bool1> output_b;

void main()
{
    int1 i1 = int1(1);
    int1 i2 = int1(2);
    int1 i3 = int1(3);
    int1 im1 = int1(-1);
    int1 im3 = int1(-3);
    uint1 u1 = uint1(1);
    uint1 u2 = uint1(2);
    uint1 u3 = uint1(3);
    uint1 um1 = uint1((uint)(-1));
    float1 fm0_5 = float1(-0.5);
    float1 f0_5 = float1(0.5);
    float1 f1 = float1(1);
    float1 f1_5 = float1(1.5);
    float1 f2 = float1(2);

    // Unary operators, except pre/post inc/dec
    // CHECK: i32 3, i32 undef
    output_i.Append(+i3); // Plus
    // CHECK: i32 -3, i32 undef
    output_i.Append(-i3); // Minus
    // CHECK: i32 -4, i32 undef
    output_i.Append(~i3); // Not
    // CHECK: i32 0, i32 undef
    output_b.Append(!i3); // LNot
    
    // CHECK: float 5.000000e-01, float undef
    output_f.Append(+f0_5); // Plus
    // CHECK: float -5.000000e-01, float undef
    output_f.Append(-f0_5); // Minus
    // CHECK: i32 0, i32 undef
    output_b.Append(!f0_5); // LNot

    // Binary operators
    // CHECK: i32 6, i32 undef
    output_i.Append(i3 * i2); // Mul
    // CHECK: i32 -1, i32 undef
    output_i.Append(im3 / i2); // Div
    // CHECK: i32 -1, i32 undef
    output_i.Append(im3 % i2); // Rem
    // CHECK: i32 3, i32 undef
    output_i.Append(i1 + i2); // Add
    // CHECK: i32 2, i32 undef
    output_i.Append(i3 - i1); // Sub

    // CHECK: float 1.000000e+00, float undef
    output_f.Append(f0_5 * f2); // Mul
    // CHECK: float 2.000000e+00, float undef
    output_f.Append(f1 / f0_5); // Div
    // CHECK: float 5.000000e-01, float undef
    output_f.Append(f2 % f1_5); // Rem
    // CHECK: float 2.000000e+00, float undef
    output_f.Append(f0_5 + f1_5); // Add
    // CHECK: float -1.000000e+00, float undef
    output_f.Append(f0_5 - f1_5); // Sub

    // CHECK: i32 6, i32 undef
    output_i.Append(i3 << i1); // Shl
    // CHECK: i32 -1, i32 undef
    output_i.Append(im1 >> i1); // Shr
    // CHECK: i32 2, i32 undef
    output_i.Append(i3 & i2); // And
    // CHECK: i32 2, i32 undef
    output_i.Append(i3 ^ i1); // Xor
    // CHECK: i32 3, i32 undef
    output_i.Append(i2 | i1); // Or

    // CHECK: i32 1, i32 undef
    output_b.Append(i3 && i2); // LAnd
    // CHECK: i32 1, i32 undef
    output_b.Append(i3 || i2); // LOr
    
    // CHECK: i32 1, i32 undef
    output_b.Append(f0_5 && f1_5); // LAnd
    // CHECK: i32 1, i32 undef
    output_b.Append(f0_5 || f1_5); // LOr

    // CHECK: i32 2147483647, i32 undef
    output_u.Append(um1 / u2); // UDiv
    // CHECK: i32 1, i32 undef
    output_u.Append(u3 % u2); // URem
    // CHECK: i32 2147483647, i32 undef
    output_u.Append(um1 >> u1); // UShr

    // CHECK: i32 1, i32 undef
    output_b.Append(im1 < i1); // LT
    // CHECK: i32 0, i32 undef
    output_b.Append(im1 > i1); // GT
    // CHECK: i32 1, i32 undef
    output_b.Append(im1 <= i1); // LE
    // CHECK: i32 0, i32 undef
    output_b.Append(im1 >= i1); // GE
    // CHECK: i32 0, i32 undef
    output_b.Append(im1 == i1); // EQ
    // CHECK: i32 1, i32 undef
    output_b.Append(im1 != i1); // NE
    // CHECK: i32 0, i32 undef
    output_b.Append(um1 < u1); // ULT
    // CHECK: i32 1, i32 undef
    output_b.Append(um1 > u1); // UGT
    // CHECK: i32 0, i32 undef
    output_b.Append(um1 <= u1); // ULE
    // CHECK: i32 1, i32 undef
    output_b.Append(um1 >= u1); // UGE
    
    // CHECK: i32 1, i32 undef
    output_b.Append(fm0_5 < f1_5); // LT
    // CHECK: i32 0, i32 undef
    output_b.Append(fm0_5 > f1_5); // GT
    // CHECK: i32 1, i32 undef
    output_b.Append(fm0_5 <= f1_5); // LE
    // CHECK: i32 0, i32 undef
    output_b.Append(fm0_5 >= f1_5); // GE
    // CHECK: i32 0, i32 undef
    output_b.Append(fm0_5 == f1_5); // EQ
    // CHECK: i32 1, i32 undef
    output_b.Append(fm0_5 != f1_5); // NE
}
