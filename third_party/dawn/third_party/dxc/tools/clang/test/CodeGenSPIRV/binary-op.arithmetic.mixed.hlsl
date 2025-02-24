// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    float4 a, b;
    float s;

    int3 c, d;
    int t;

    float1 e, f;
    int1 g, h;

    float2x3 i, j;
    float1x3 k, l;
    float2x1 m, n;
    float1x1 o, p;

    // Use OpVectorTimesScalar for floatN * float
// CHECK:      [[a4:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[s0:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpVectorTimesScalar %v4float [[a4]] [[s0]]
// CHECK-NEXT: OpStore %b [[mul0]]
    b = a * s;
// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[s1:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul1:%[0-9]+]] = OpVectorTimesScalar %v4float [[a5]] [[s1]]
// CHECK-NEXT: OpStore %b [[mul1]]
    b = s * a;

    // Use normal OpCompositeConstruct and OpIMul for intN * int
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %v3int %c
// CHECK-NEXT: [[t0:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[cc10:%[0-9]+]] = OpCompositeConstruct %v3int [[t0]] [[t0]] [[t0]]
// CHECK-NEXT: [[mul2:%[0-9]+]] = OpIMul %v3int [[c0]] [[cc10]]
// CHECK-NEXT: OpStore %d [[mul2]]
    d = c * t;
// CHECK-NEXT: [[t1:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[cc11:%[0-9]+]] = OpCompositeConstruct %v3int [[t1]] [[t1]] [[t1]]
// CHECK-NEXT: [[c1:%[0-9]+]] = OpLoad %v3int %c
// CHECK-NEXT: [[mul3:%[0-9]+]] = OpIMul %v3int [[cc11]] [[c1]]
// CHECK-NEXT: OpStore %d [[mul3]]
    d = t * c;

    // Vector of size 1
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: [[s2:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul4:%[0-9]+]] = OpFMul %float [[e0]] [[s2]]
// CHECK-NEXT: OpStore %f [[mul4]]
    f = e * s;
// CHECK-NEXT: [[s3:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[e1:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: [[mul5:%[0-9]+]] = OpFMul %float [[s3]] [[e1]]
// CHECK-NEXT: OpStore %f [[mul5]]
    f = s * e;
// CHECK-NEXT: [[g0:%[0-9]+]] = OpLoad %int %g
// CHECK-NEXT: [[t2:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[mul6:%[0-9]+]] = OpIMul %int [[g0]] [[t2]]
// CHECK-NEXT: OpStore %h [[mul6]]
    h = g * t;
// CHECK-NEXT: [[t3:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[g1:%[0-9]+]] = OpLoad %int %g
// CHECK-NEXT: [[mul7:%[0-9]+]] = OpIMul %int [[t3]] [[g1]]
// CHECK-NEXT: OpStore %h [[mul7]]
    h = t * g;

    // Use OpMatrixTimesScalar for floatMxN * float
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %mat2v3float %i
// CHECK-NEXT: [[s4:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul8:%[0-9]+]] = OpMatrixTimesScalar %mat2v3float [[i0]] [[s4]]
// CHECK-NEXT: OpStore %j [[mul8]]
    j = i * s;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %mat2v3float %i
// CHECK-NEXT: [[s5:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul9:%[0-9]+]] = OpMatrixTimesScalar %mat2v3float [[i1]] [[s5]]
// CHECK-NEXT: OpStore %j [[mul9]]
    j = s * i;

    // Use OpVectorTimesScalar for float1xN * float
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %v3float %k
// CHECK-NEXT: [[s6:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul10:%[0-9]+]] = OpVectorTimesScalar %v3float [[k0]] [[s6]]
// CHECK-NEXT: OpStore %l [[mul10]]
    l = k * s;
// CHECK-NEXT: [[k1:%[0-9]+]] = OpLoad %v3float %k
// CHECK-NEXT: [[s7:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul11:%[0-9]+]] = OpVectorTimesScalar %v3float [[k1]] [[s7]]
// CHECK-NEXT: OpStore %l [[mul11]]
    l = s * k;

    // Use OpVectorTimesScalar for floatMx1 * float
// CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %v2float %m
// CHECK-NEXT: [[s8:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul12:%[0-9]+]] = OpVectorTimesScalar %v2float [[m0]] [[s8]]
// CHECK-NEXT: OpStore %n [[mul12]]
    n = m * s;
// CHECK-NEXT: [[m1:%[0-9]+]] = OpLoad %v2float %m
// CHECK-NEXT: [[s9:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul13:%[0-9]+]] = OpVectorTimesScalar %v2float [[m1]] [[s9]]
// CHECK-NEXT: OpStore %n [[mul13]]
    n = s * m;

    // Matrix of size 1x1
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[s10:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[mul14:%[0-9]+]] = OpFMul %float [[o0]] [[s10]]
// CHECK-NEXT: OpStore %p [[mul14]]
    p = o * s;
// CHECK-NEXT: [[s11:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[o1:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[mul15:%[0-9]+]] = OpFMul %float [[s11]] [[o1]]
// CHECK-NEXT: OpStore %p [[mul15]]
    p = s * o;

// Non-floating point matrices:
// Since non-fp matrices are represented as arrays of vectors, we cannot use
// OpMatrixTimes* instructions.

    int2x3 q;

// Note: The AST includes a MatrixSplat, therefore we splat the scalar to a matrix. So we cannot use OpVectorTimesScalar.
// CHECK:          [[t:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT:  [[tvec:%[0-9]+]] = OpCompositeConstruct %v3int [[t]] [[t]] [[t]]
// CHECK-NEXT:  [[tmat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[tvec]] [[tvec]]
// CHECK-NEXT:     [[q:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %q
// CHECK-NEXT: [[tmat0:%[0-9]+]] = OpCompositeExtract %v3int [[tmat]] 0
// CHECK-NEXT:    [[q0:%[0-9]+]] = OpCompositeExtract %v3int [[q]] 0
// CHECK-NEXT:   [[qt0:%[0-9]+]] = OpIMul %v3int [[tmat0]] [[q0]]
// CHECK-NEXT: [[tmat1:%[0-9]+]] = OpCompositeExtract %v3int [[tmat]] 1
// CHECK-NEXT:    [[q1:%[0-9]+]] = OpCompositeExtract %v3int [[q]] 1
// CHECK-NEXT:   [[qt1:%[0-9]+]] = OpIMul %v3int [[tmat1]] [[q1]]
// CHECK-NEXT:    [[qt:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[qt0]] [[qt1]]
// CHECK-NEXT:                  OpStore %qt [[qt]]
    int2x3 qt = t * q;

    bool2x3 x;

// Note: The AST includes a MatrixSplat, therefore we splat the scalar to a matrix. So we cannot use OpVectorTimesScalar.
// CHECK:                [[z:%[0-9]+]] = OpLoad %bool %z
// CHECK-NEXT:        [[zint:%[0-9]+]] = OpSelect %int [[z]] %int_1 %int_0
// CHECK-NEXT:        [[zvec:%[0-9]+]] = OpCompositeConstruct %v3int [[zint]] [[zint]] [[zint]]
// CHECK-NEXT:   [[z_int_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[zvec]] [[zvec]]
// CHECK-NEXT:           [[x:%[0-9]+]] = OpLoad %_arr_v3bool_uint_2 %x
// CHECK-NEXT:          [[x0:%[0-9]+]] = OpCompositeExtract %v3bool [[x]] 0
// CHECK-NEXT:       [[x0int:%[0-9]+]] = OpSelect %v3int [[x0]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:          [[x1:%[0-9]+]] = OpCompositeExtract %v3bool [[x]] 1
// CHECK-NEXT:       [[x1int:%[0-9]+]] = OpSelect %v3int [[x1]] {{%[0-9]+}} {{%[0-9]+}}
// CHECK-NEXT:   [[x_int_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[x0int]] [[x1int]]
// CHECK-NEXT:          [[z0:%[0-9]+]] = OpCompositeExtract %v3int [[z_int_mat]] 0
// CHECK-NEXT:          [[x0_0:%[0-9]+]] = OpCompositeExtract %v3int [[x_int_mat]] 0
// CHECK-NEXT:         [[zx0:%[0-9]+]] = OpIMul %v3int [[z0]] [[x0_0]]
// CHECK-NEXT:          [[z1:%[0-9]+]] = OpCompositeExtract %v3int [[z_int_mat]] 1
// CHECK-NEXT:          [[x1_0:%[0-9]+]] = OpCompositeExtract %v3int [[x_int_mat]] 1
// CHECK-NEXT:         [[zx1:%[0-9]+]] = OpIMul %v3int [[z1]] [[x1_0]]
// CHECK-NEXT:  [[zx_int_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[zx0]] [[zx1]]
// CHECK-NEXT:         [[zx0_0:%[0-9]+]] = OpCompositeExtract %v3int [[zx_int_mat]] 0
// CHECK-NEXT:     [[zx0bool:%[0-9]+]] = OpINotEqual %v3bool [[zx0_0]] {{%[0-9]+}}
// CHECK-NEXT:         [[zx1_0:%[0-9]+]] = OpCompositeExtract %v3int [[zx_int_mat]] 1
// CHECK-NEXT:     [[zx1bool:%[0-9]+]] = OpINotEqual %v3bool [[zx1_0]] {{%[0-9]+}}
// CHECK-NEXT: [[zx_bool_mat:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[zx0bool]] [[zx1bool]]
// CHECK-NEXT:                        OpStore %zx [[zx_bool_mat]]
    bool z;
    bool2x3 zx = z * x;
}
