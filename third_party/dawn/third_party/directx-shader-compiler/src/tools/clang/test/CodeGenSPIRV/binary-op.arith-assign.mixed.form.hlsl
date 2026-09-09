// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
    float4 a;
    float s;

    int3 c;
    int t;

    float1 e;
    int1 g;

    float2x3 i;
    float1x3 k;
    float2x1 m;
    float1x1 o;

    // Use OpVectorTimesScalar for floatN * float
// CHECK:      [[s0:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[a0:%[0-9]+]] = OpLoad %v4float %a
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpVectorTimesScalar %v4float [[a0]] [[s0]]
// CHECK-NEXT: OpStore %a [[mul0]]
    a *= s;

    // Use normal OpCompositeConstruct and OpIMul for intN * int
// CHECK-NEXT: [[t0:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[cc0:%[0-9]+]] = OpCompositeConstruct %v3int [[t0]] [[t0]] [[t0]]
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %v3int %c
// CHECK-NEXT: [[mul2:%[0-9]+]] = OpIMul %v3int [[c0]] [[cc0]]
// CHECK-NEXT: OpStore %c [[mul2]]
    c *= t;

    // Vector of size 1
// CHECK-NEXT: [[s2:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %float %e
// CHECK-NEXT: [[mul4:%[0-9]+]] = OpFMul %float [[e0]] [[s2]]
// CHECK-NEXT: OpStore %e [[mul4]]
    e *= s;
// CHECK-NEXT: [[t2:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT: [[g0:%[0-9]+]] = OpLoad %int %g
// CHECK-NEXT: [[mul6:%[0-9]+]] = OpIMul %int [[g0]] [[t2]]
// CHECK-NEXT: OpStore %g [[mul6]]
    g *= t;

    // Use OpMatrixTimesScalar for floatMxN * float
// CHECK-NEXT: [[s4:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %mat2v3float %i
// CHECK-NEXT: [[mul8:%[0-9]+]] = OpMatrixTimesScalar %mat2v3float [[i0]] [[s4]]
// CHECK-NEXT: OpStore %i [[mul8]]
    i *= s;

    // Use OpVectorTimesScalar for float1xN * float
// CHECK-NEXT: [[s6:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %v3float %k
// CHECK-NEXT: [[mul10:%[0-9]+]] = OpVectorTimesScalar %v3float [[k0]] [[s6]]
// CHECK-NEXT: OpStore %k [[mul10]]
    k *= s;

    // Use OpVectorTimesScalar for floatMx1 * float
// CHECK-NEXT: [[s8:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %v2float %m
// CHECK-NEXT: [[mul12:%[0-9]+]] = OpVectorTimesScalar %v2float [[m0]] [[s8]]
// CHECK-NEXT: OpStore %m [[mul12]]
    m *= s;

    // Matrix of size 1x1
// CHECK-NEXT: [[s10:%[0-9]+]] = OpLoad %float %s
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %float %o
// CHECK-NEXT: [[mul14:%[0-9]+]] = OpFMul %float [[o0]] [[s10]]
// CHECK-NEXT: OpStore %o [[mul14]]
    o *= s;

// Non-floating point matrices

    int2x3 p;

// Note: The AST includes a MatrixSplat, therefore we splat the scalar to a matrix. So we cannot use OpVectorTimesScalar.
// CHECK-NEXT:      [[t:%[0-9]+]] = OpLoad %int %t
// CHECK-NEXT:   [[tvec:%[0-9]+]] = OpCompositeConstruct %v3int [[t]] [[t]] [[t]]
// CHECK-NEXT:   [[tmat:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[tvec]] [[tvec]]
// CHECK-NEXT:      [[p:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %p
// CHECK-NEXT:     [[p0:%[0-9]+]] = OpCompositeExtract %v3int [[p]] 0
// CHECK-NEXT:  [[tmat0:%[0-9]+]] = OpCompositeExtract %v3int [[tmat]] 0
// CHECK-NEXT: [[new_p0:%[0-9]+]] = OpIMul %v3int [[p0]] [[tmat0]]
// CHECK-NEXT:     [[p1:%[0-9]+]] = OpCompositeExtract %v3int [[p]] 1
// CHECK-NEXT:  [[tmat1:%[0-9]+]] = OpCompositeExtract %v3int [[tmat]] 1
// CHECK-NEXT: [[new_p1:%[0-9]+]] = OpIMul %v3int [[p1]] [[tmat1]]
// CHECK-NEXT:  [[new_p:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[new_p0]] [[new_p1]]
// CHECK-NEXT:                   OpStore %p [[new_p]]
    p *= t;

// Note: Boolean matrix not allowed by the front-end for these operations.
}
