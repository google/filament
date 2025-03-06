// RUN: %dxc -E main -T vs_6_0 -HV 2018 %s | FileCheck %s

// Tests the implementation of unary and binary matrix operators

// Workaround for AppendStructuredBuffer<matrix>.Append bug
#define Append(buf, val) buf[buf.IncrementCounter()] = (val)

RWStructuredBuffer<int1x1> output_i;
RWStructuredBuffer<uint1x1> output_u;
RWStructuredBuffer<float1x1> output_f;
RWStructuredBuffer<bool1x1> output_b;

void main()
{
    int1x1 i1 = int1x1(1);
    int1x1 i2 = int1x1(2);
    int1x1 i3 = int1x1(3);
    int1x1 im1 = int1x1(-1);
    int1x1 im3 = int1x1(-3);
    uint1x1 u1 = uint1x1(1);
    uint1x1 u2 = uint1x1(2);
    uint1x1 u3 = uint1x1(3);
    uint1x1 um1 = uint1x1((uint)(-1));
    float1x1 fm0_5 = float1x1(-0.5);
    float1x1 f0_5 = float1x1(0.5);
    float1x1 f1 = float1x1(1);
    float1x1 f1_5 = float1x1(1.5);
    float1x1 f2 = float1x1(2);

    // Unary operators, except pre/post inc/dec
    // CHECK: i32 3, i32 undef
    Append(output_i, +i3); // Plus
    // CHECK: i32 -3, i32 undef
    Append(output_i, -i3); // Minus
    // CHECK: i32 -4, i32 undef
    Append(output_i, ~i3); // Not
    // CHECK: i32 0, i32 undef
    Append(output_b, !i3); // LNot
    
    // CHECK: float 5.000000e-01, float undef
    Append(output_f, +f0_5); // Plus
    // CHECK: float -5.000000e-01, float undef
    Append(output_f, -f0_5); // Minus
    // CHECK: i32 0, i32 undef
    Append(output_b, !f0_5); // LNot

    // Binary operators
    // CHECK: i32 6, i32 undef
    Append(output_i, i3 * i2); // Mul
    // CHECK: i32 -1, i32 undef
    Append(output_i, im3 / i2); // Div
    // CHECK: i32 -1, i32 undef
    Append(output_i, im3 % i2); // Rem
    // CHECK: i32 3, i32 undef
    Append(output_i, i1 + i2); // Add
    // CHECK: i32 2, i32 undef
    Append(output_i, i3 - i1); // Sub

    // CHECK: float 1.000000e+00, float undef
    Append(output_f, f0_5 * f2); // Mul
    // CHECK: float 2.000000e+00, float undef
    Append(output_f, f1 / f0_5); // Div
    // CHECK: float 5.000000e-01, float undef
    Append(output_f, f2 % f1_5); // Rem
    // CHECK: float 2.000000e+00, float undef
    Append(output_f, f0_5 + f1_5); // Add
    // CHECK: float -1.000000e+00, float undef
    Append(output_f, f0_5 - f1_5); // Sub

    // CHECK: i32 6, i32 undef
    Append(output_i, i3 << i1); // Shl
    // CHECK: i32 -1, i32 undef
    Append(output_i, im1 >> i1); // Shr
    // CHECK: i32 2, i32 undef
    Append(output_i, i3 & i2); // And
    // CHECK: i32 2, i32 undef
    Append(output_i, i3 ^ i1); // Xor
    // CHECK: i32 3, i32 undef
    Append(output_i, i2 | i1); // Or

    // CHECK: i32 1, i32 undef
    Append(output_b, i3 && i2); // LAnd
    // CHECK: i32 1, i32 undef
    Append(output_b, i3 || i2); // LOr
    
    // CHECK: i32 1, i32 undef
    Append(output_b, f0_5 && f1_5); // LAnd
    // CHECK: i32 1, i32 undef
    Append(output_b, f0_5 || f1_5); // LOr

    // CHECK: i32 2147483647, i32 undef
    Append(output_u, um1 / u2); // UDiv
    // CHECK: i32 1, i32 undef
    Append(output_u, u3 % u2); // URem
    // CHECK: i32 2147483647, i32 undef
    Append(output_u, um1 >> u1); // UShr

    // CHECK: i32 1, i32 undef
    Append(output_b, im1 < i1); // LT
    // CHECK: i32 0, i32 undef
    Append(output_b, im1 > i1); // GT
    // CHECK: i32 1, i32 undef
    Append(output_b, im1 <= i1); // LE
    // CHECK: i32 0, i32 undef
    Append(output_b, im1 >= i1); // GE
    // CHECK: i32 0, i32 undef
    Append(output_b, im1 == i1); // EQ
    // CHECK: i32 1, i32 undef
    Append(output_b, im1 != i1); // NE
    // CHECK: i32 0, i32 undef
    Append(output_b, um1 < u1); // ULT
    // CHECK: i32 1, i32 undef
    Append(output_b, um1 > u1); // UGT
    // CHECK: i32 0, i32 undef
    Append(output_b, um1 <= u1); // ULE
    // CHECK: i32 1, i32 undef
    Append(output_b, um1 >= u1); // UGE
    
    // CHECK: i32 1, i32 undef
    Append(output_b, fm0_5 < f1_5); // LT
    // CHECK: i32 0, i32 undef
    Append(output_b, fm0_5 > f1_5); // GT
    // CHECK: i32 1, i32 undef
    Append(output_b, fm0_5 <= f1_5); // LE
    // CHECK: i32 0, i32 undef
    Append(output_b, fm0_5 >= f1_5); // GE
    // CHECK: i32 0, i32 undef
    Append(output_b, fm0_5 == f1_5); // EQ
    // CHECK: i32 1, i32 undef
    Append(output_b, fm0_5 != f1_5); // NE
}
