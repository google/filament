// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3int1:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_1 %int_1
// CHECK: [[v3int0:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // 1x1
    float1x1 a, b, c;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[c0:%[0-9]+]] = OpFAdd %float [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[c0]]
    c = a + b;
// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[c1:%[0-9]+]] = OpFSub %float [[a1]] [[b1]]
// CHECK-NEXT: OpStore %c [[c1]]
    c = a - b;
// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[c2:%[0-9]+]] = OpFMul %float [[a2]] [[b2]]
// CHECK-NEXT: OpStore %c [[c2]]
    c = a * b;
// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b3:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[c3:%[0-9]+]] = OpFDiv %float [[a3]] [[b3]]
// CHECK-NEXT: OpStore %c [[c3]]
    c = a / b;
// CHECK-NEXT: [[a4:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b4:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[c4:%[0-9]+]] = OpFRem %float [[a4]] [[b4]]
// CHECK-NEXT: OpStore %c [[c4]]
    c = a % b;

    // Mx1
    float2x1 h, i, j;
// CHECK-NEXT: [[h0:%[0-9]+]] = OpLoad %v2float %h
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %v2float %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpFAdd %v2float [[h0]] [[i0]]
// CHECK-NEXT: OpStore %j [[j0]]
    j = h + i;
// CHECK-NEXT: [[h1:%[0-9]+]] = OpLoad %v2float %h
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %v2float %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpFSub %v2float [[h1]] [[i1]]
// CHECK-NEXT: OpStore %j [[j1]]
    j = h - i;
// CHECK-NEXT: [[h2:%[0-9]+]] = OpLoad %v2float %h
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %v2float %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpFMul %v2float [[h2]] [[i2]]
// CHECK-NEXT: OpStore %j [[j2]]
    j = h * i;
// CHECK-NEXT: [[h3:%[0-9]+]] = OpLoad %v2float %h
// CHECK-NEXT: [[i3:%[0-9]+]] = OpLoad %v2float %i
// CHECK-NEXT: [[j3:%[0-9]+]] = OpFDiv %v2float [[h3]] [[i3]]
// CHECK-NEXT: OpStore %j [[j3]]
    j = h / i;
// CHECK-NEXT: [[h4:%[0-9]+]] = OpLoad %v2float %h
// CHECK-NEXT: [[i4:%[0-9]+]] = OpLoad %v2float %i
// CHECK-NEXT: [[j4:%[0-9]+]] = OpFRem %v2float [[h4]] [[i4]]
// CHECK-NEXT: OpStore %j [[j4]]
    j = h % i;

    // 1xN
    float1x3 o, p, q;
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v3float %p
// CHECK-NEXT: [[q0:%[0-9]+]] = OpFAdd %v3float [[o0]] [[p0]]
// CHECK-NEXT: OpStore %q [[q0]]
    q = o + p;
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT: [[p1:%[0-9]+]] = OpLoad %v3float %p
// CHECK-NEXT: [[q1:%[0-9]+]] = OpFSub %v3float [[o1]] [[p1]]
// CHECK-NEXT: OpStore %q [[q1]]
    q = o - p;
// CHECK-NEXT: [[o2:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT: [[p2:%[0-9]+]] = OpLoad %v3float %p
// CHECK-NEXT: [[q2:%[0-9]+]] = OpFMul %v3float [[o2]] [[p2]]
// CHECK-NEXT: OpStore %q [[q2]]
    q = o * p;
// CHECK-NEXT: [[o3:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT: [[p3:%[0-9]+]] = OpLoad %v3float %p
// CHECK-NEXT: [[q3:%[0-9]+]] = OpFDiv %v3float [[o3]] [[p3]]
// CHECK-NEXT: OpStore %q [[q3]]
    q = o / p;
// CHECK-NEXT: [[o4:%[0-9]+]] = OpLoad %v3float %o
// CHECK-NEXT: [[p4:%[0-9]+]] = OpLoad %v3float %p
// CHECK-NEXT: [[q4:%[0-9]+]] = OpFRem %v3float [[o4]] [[p4]]
// CHECK-NEXT: OpStore %q [[q4]]
    q = o % p;

    // MxN
    float2x3 r, s, t;
// CHECK-NEXT: [[r0:%[0-9]+]] = OpLoad %mat2v3float %r
// CHECK-NEXT: [[s0:%[0-9]+]] = OpLoad %mat2v3float %s
// CHECK-NEXT: [[r0v0:%[0-9]+]] = OpCompositeExtract %v3float [[r0]] 0
// CHECK-NEXT: [[s0v0:%[0-9]+]] = OpCompositeExtract %v3float [[s0]] 0
// CHECK-NEXT: [[t0v0:%[0-9]+]] = OpFAdd %v3float [[r0v0]] [[s0v0]]
// CHECK-NEXT: [[r0v1:%[0-9]+]] = OpCompositeExtract %v3float [[r0]] 1
// CHECK-NEXT: [[s0v1:%[0-9]+]] = OpCompositeExtract %v3float [[s0]] 1
// CHECK-NEXT: [[t0v1:%[0-9]+]] = OpFAdd %v3float [[r0v1]] [[s0v1]]
// CHECK-NEXT: [[t0:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[t0v0]] [[t0v1]]
// CHECK-NEXT: OpStore %t [[t0]]
    t = r + s;
// CHECK-NEXT: [[r1:%[0-9]+]] = OpLoad %mat2v3float %r
// CHECK-NEXT: [[s1:%[0-9]+]] = OpLoad %mat2v3float %s
// CHECK-NEXT: [[r1v0:%[0-9]+]] = OpCompositeExtract %v3float [[r1]] 0
// CHECK-NEXT: [[s1v0:%[0-9]+]] = OpCompositeExtract %v3float [[s1]] 0
// CHECK-NEXT: [[t1v0:%[0-9]+]] = OpFSub %v3float [[r1v0]] [[s1v0]]
// CHECK-NEXT: [[r1v1:%[0-9]+]] = OpCompositeExtract %v3float [[r1]] 1
// CHECK-NEXT: [[s1v1:%[0-9]+]] = OpCompositeExtract %v3float [[s1]] 1
// CHECK-NEXT: [[t1v1:%[0-9]+]] = OpFSub %v3float [[r1v1]] [[s1v1]]
// CHECK-NEXT: [[t1:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[t1v0]] [[t1v1]]
// CHECK-NEXT: OpStore %t [[t1]]
    t = r - s;
// CHECK-NEXT: [[r2:%[0-9]+]] = OpLoad %mat2v3float %r
// CHECK-NEXT: [[s2:%[0-9]+]] = OpLoad %mat2v3float %s
// CHECK-NEXT: [[r2v0:%[0-9]+]] = OpCompositeExtract %v3float [[r2]] 0
// CHECK-NEXT: [[s2v0:%[0-9]+]] = OpCompositeExtract %v3float [[s2]] 0
// CHECK-NEXT: [[t2v0:%[0-9]+]] = OpFMul %v3float [[r2v0]] [[s2v0]]
// CHECK-NEXT: [[r2v1:%[0-9]+]] = OpCompositeExtract %v3float [[r2]] 1
// CHECK-NEXT: [[s2v1:%[0-9]+]] = OpCompositeExtract %v3float [[s2]] 1
// CHECK-NEXT: [[t2v1:%[0-9]+]] = OpFMul %v3float [[r2v1]] [[s2v1]]
// CHECK-NEXT: [[t2:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[t2v0]] [[t2v1]]
// CHECK-NEXT: OpStore %t [[t2]]
    t = r * s;
// CHECK-NEXT: [[r3:%[0-9]+]] = OpLoad %mat2v3float %r
// CHECK-NEXT: [[s3:%[0-9]+]] = OpLoad %mat2v3float %s
// CHECK-NEXT: [[r3v0:%[0-9]+]] = OpCompositeExtract %v3float [[r3]] 0
// CHECK-NEXT: [[s3v0:%[0-9]+]] = OpCompositeExtract %v3float [[s3]] 0
// CHECK-NEXT: [[t3v0:%[0-9]+]] = OpFDiv %v3float [[r3v0]] [[s3v0]]
// CHECK-NEXT: [[r3v1:%[0-9]+]] = OpCompositeExtract %v3float [[r3]] 1
// CHECK-NEXT: [[s3v1:%[0-9]+]] = OpCompositeExtract %v3float [[s3]] 1
// CHECK-NEXT: [[t3v1:%[0-9]+]] = OpFDiv %v3float [[r3v1]] [[s3v1]]
// CHECK-NEXT: [[t3:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[t3v0]] [[t3v1]]
// CHECK-NEXT: OpStore %t [[t3]]
    t = r / s;
// CHECK-NEXT: [[r4:%[0-9]+]] = OpLoad %mat2v3float %r
// CHECK-NEXT: [[s4:%[0-9]+]] = OpLoad %mat2v3float %s
// CHECK-NEXT: [[r4v0:%[0-9]+]] = OpCompositeExtract %v3float [[r4]] 0
// CHECK-NEXT: [[s4v0:%[0-9]+]] = OpCompositeExtract %v3float [[s4]] 0
// CHECK-NEXT: [[t4v0:%[0-9]+]] = OpFRem %v3float [[r4v0]] [[s4v0]]
// CHECK-NEXT: [[r4v1:%[0-9]+]] = OpCompositeExtract %v3float [[r4]] 1
// CHECK-NEXT: [[s4v1:%[0-9]+]] = OpCompositeExtract %v3float [[s4]] 1
// CHECK-NEXT: [[t4v1:%[0-9]+]] = OpFRem %v3float [[r4v1]] [[s4v1]]
// CHECK-NEXT: [[t4:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[t4v0]] [[t4v1]]
// CHECK-NEXT: OpStore %t [[t4]]
    t = r % s;

    // MxN non-floating point matrices
    int2x3 u, v, w;
// CHECK-NEXT: [[u0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %u
// CHECK-NEXT: [[v0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %v
// CHECK-NEXT: [[u0v0:%[0-9]+]] = OpCompositeExtract %v3int [[u0]] 0
// CHECK-NEXT: [[v0v0:%[0-9]+]] = OpCompositeExtract %v3int [[v0]] 0
// CHECK-NEXT: [[w0v0:%[0-9]+]] = OpIAdd %v3int [[u0v0]] [[v0v0]]
// CHECK-NEXT: [[u0v1:%[0-9]+]] = OpCompositeExtract %v3int [[u0]] 1
// CHECK-NEXT: [[v0v1:%[0-9]+]] = OpCompositeExtract %v3int [[v0]] 1
// CHECK-NEXT: [[w0v1:%[0-9]+]] = OpIAdd %v3int [[u0v1]] [[v0v1]]
// CHECK-NEXT: [[w0:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[w0v0]] [[w0v1]]
// CHECK-NEXT: OpStore %w [[w0]]
    w = u + v;
// CHECK-NEXT: [[u1:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %u
// CHECK-NEXT: [[v1:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %v
// CHECK-NEXT: [[u1v0:%[0-9]+]] = OpCompositeExtract %v3int [[u1]] 0
// CHECK-NEXT: [[v1v0:%[0-9]+]] = OpCompositeExtract %v3int [[v1]] 0
// CHECK-NEXT: [[w1v0:%[0-9]+]] = OpISub %v3int [[u1v0]] [[v1v0]]
// CHECK-NEXT: [[u1v1:%[0-9]+]] = OpCompositeExtract %v3int [[u1]] 1
// CHECK-NEXT: [[v1v1:%[0-9]+]] = OpCompositeExtract %v3int [[v1]] 1
// CHECK-NEXT: [[w1v1:%[0-9]+]] = OpISub %v3int [[u1v1]] [[v1v1]]
// CHECK-NEXT: [[w1:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[w1v0]] [[w1v1]]
// CHECK-NEXT: OpStore %w [[w1]]
    w = u - v;
// CHECK-NEXT: [[u2:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %u
// CHECK-NEXT: [[v2:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %v
// CHECK-NEXT: [[u2v0:%[0-9]+]] = OpCompositeExtract %v3int [[u2]] 0
// CHECK-NEXT: [[v2v0:%[0-9]+]] = OpCompositeExtract %v3int [[v2]] 0
// CHECK-NEXT: [[w2v0:%[0-9]+]] = OpIMul %v3int [[u2v0]] [[v2v0]]
// CHECK-NEXT: [[u2v1:%[0-9]+]] = OpCompositeExtract %v3int [[u2]] 1
// CHECK-NEXT: [[v2v1:%[0-9]+]] = OpCompositeExtract %v3int [[v2]] 1
// CHECK-NEXT: [[w2v1:%[0-9]+]] = OpIMul %v3int [[u2v1]] [[v2v1]]
// CHECK-NEXT: [[w2:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[w2v0]] [[w2v1]]
// CHECK-NEXT: OpStore %w [[w2]]
    w = u * v;
// CHECK-NEXT: [[u3:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %u
// CHECK-NEXT: [[v3:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %v
// CHECK-NEXT: [[u3v0:%[0-9]+]] = OpCompositeExtract %v3int [[u3]] 0
// CHECK-NEXT: [[v3v0:%[0-9]+]] = OpCompositeExtract %v3int [[v3]] 0
// CHECK-NEXT: [[w3v0:%[0-9]+]] = OpSDiv %v3int [[u3v0]] [[v3v0]]
// CHECK-NEXT: [[u3v1:%[0-9]+]] = OpCompositeExtract %v3int [[u3]] 1
// CHECK-NEXT: [[v3v1:%[0-9]+]] = OpCompositeExtract %v3int [[v3]] 1
// CHECK-NEXT: [[w3v1:%[0-9]+]] = OpSDiv %v3int [[u3v1]] [[v3v1]]
// CHECK-NEXT: [[w3:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[w3v0]] [[w3v1]]
// CHECK-NEXT: OpStore %w [[w3]]
    w = u / v;
// CHECK-NEXT: [[u4:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %u
// CHECK-NEXT: [[v4:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %v
// CHECK-NEXT: [[u4v0:%[0-9]+]] = OpCompositeExtract %v3int [[u4]] 0
// CHECK-NEXT: [[v4v0:%[0-9]+]] = OpCompositeExtract %v3int [[v4]] 0
// CHECK-NEXT: [[w4v0:%[0-9]+]] = OpSRem %v3int [[u4v0]] [[v4v0]]
// CHECK-NEXT: [[u4v1:%[0-9]+]] = OpCompositeExtract %v3int [[u4]] 1
// CHECK-NEXT: [[v4v1:%[0-9]+]] = OpCompositeExtract %v3int [[v4]] 1
// CHECK-NEXT: [[w4v1:%[0-9]+]] = OpSRem %v3int [[u4v1]] [[v4v1]]
// CHECK-NEXT: [[w4:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[w4v0]] [[w4v1]]
// CHECK-NEXT: OpStore %w [[w4]]
    w = u % v;

    // Boolean matrices
    // In all cases, the boolean matrix (represented as an array of boolean vectores)
    // is first casted to an integer matrix (represented as an array of integer vectors).
    // Then, the binary operation (e.g. '+', '-', '*', '/', '%') is performed and then
    // it is converted back to a boolean matrix. This behavior is due to the AST.
    bool2x3 x, y, z;
// CHECK-NEXT:      [[x0:%[0-9]+]] = OpLoad %_arr_v3bool_uint_2 %x
// CHECK-NEXT:    [[x0v0:%[0-9]+]] = OpCompositeExtract %v3bool [[x0]] 0
// CHECK-NEXT: [[x0v0int:%[0-9]+]] = OpSelect %v3int [[x0v0]] [[v3int1]] [[v3int0]]
// CHECK-NEXT:    [[x0v1:%[0-9]+]] = OpCompositeExtract %v3bool [[x0]] 1
// CHECK-NEXT: [[x0v1int:%[0-9]+]] = OpSelect %v3int [[x0v1]] [[v3int1]] [[v3int0]]
// CHECK-NEXT:   [[x0int:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[x0v0int]] [[x0v1int]]
// CHECK-NEXT:      [[y0:%[0-9]+]] = OpLoad %_arr_v3bool_uint_2 %y
// CHECK-NEXT:    [[y0v0:%[0-9]+]] = OpCompositeExtract %v3bool [[y0]] 0
// CHECK-NEXT: [[y0v0int:%[0-9]+]] = OpSelect %v3int [[y0v0]] [[v3int1]] [[v3int0]]
// CHECK-NEXT:    [[y0v1:%[0-9]+]] = OpCompositeExtract %v3bool [[y0]] 1
// CHECK-NEXT: [[y0v1int:%[0-9]+]] = OpSelect %v3int [[y0v1]] [[v3int1]] [[v3int0]]
// CHECK-NEXT:   [[y0int:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[y0v0int]] [[y0v1int]]
// CHECK-NEXT:    [[x0v0_0:%[0-9]+]] = OpCompositeExtract %v3int [[x0int]] 0
// CHECK-NEXT:    [[y0v0_0:%[0-9]+]] = OpCompositeExtract %v3int [[y0int]] 0
// CHECK-NEXT:    [[z0v0:%[0-9]+]] = OpIAdd %v3int [[x0v0_0]] [[y0v0_0]]
// CHECK-NEXT:    [[x0v1_0:%[0-9]+]] = OpCompositeExtract %v3int [[x0int]] 1
// CHECK-NEXT:    [[y0v1_0:%[0-9]+]] = OpCompositeExtract %v3int [[y0int]] 1
// CHECK-NEXT:    [[z0v1:%[0-9]+]] = OpIAdd %v3int [[x0v1_0]] [[y0v1_0]]
// CHECK-NEXT:   [[z_int:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[z0v0]] [[z0v1]]
// CHECK-NEXT:    [[z0v0_0:%[0-9]+]] = OpCompositeExtract %v3int [[z_int]] 0
// CHECK-NEXT:[[z0v0bool:%[0-9]+]] = OpINotEqual %v3bool [[z0v0_0]] [[v3int0]]
// CHECK-NEXT:    [[z0v1_0:%[0-9]+]] = OpCompositeExtract %v3int [[z_int]] 1
// CHECK-NEXT:[[z0v1bool:%[0-9]+]] = OpINotEqual %v3bool [[z0v1_0]] [[v3int0]]
// CHECK-NEXT:       [[z:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[z0v0bool]] [[z0v1bool]]
// CHECK-NEXT:                    OpStore %z [[z]]
    z = x + y;
}
