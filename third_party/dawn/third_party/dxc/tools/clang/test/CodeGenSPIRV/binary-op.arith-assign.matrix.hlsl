// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    float1x1 a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[b1:%[0-9]+]] = OpFAdd %float [[b0]] [[a0]]
// CHECK-NEXT: OpStore %b [[b1]]
    b += a;

    float2x1 c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %v2float %c
// CHECK-NEXT: [[d0:%[0-9]+]] = OpLoad %v2float %d
// CHECK-NEXT: [[d1:%[0-9]+]] = OpFSub %v2float [[d0]] [[c0]]
// CHECK-NEXT: OpStore %d [[d1]]
    d -= c;

    float1x3 e, f;
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %v3float %e
// CHECK-NEXT: [[f0:%[0-9]+]] = OpLoad %v3float %f
// CHECK-NEXT: [[f1:%[0-9]+]] = OpFMul %v3float [[f0]] [[e0]]
// CHECK-NEXT: OpStore %f [[f1]]
    f *= e;

    float2x3 g, h;
// CHECK-NEXT: [[g0:%[0-9]+]] = OpLoad %mat2v3float %g
// CHECK-NEXT: [[h0:%[0-9]+]] = OpLoad %mat2v3float %h
// CHECK-NEXT: [[h0v0:%[0-9]+]] = OpCompositeExtract %v3float [[h0]] 0
// CHECK-NEXT: [[g0v0:%[0-9]+]] = OpCompositeExtract %v3float [[g0]] 0
// CHECK-NEXT: [[h1v0:%[0-9]+]] = OpFDiv %v3float [[h0v0]] [[g0v0]]
// CHECK-NEXT: [[h0v1:%[0-9]+]] = OpCompositeExtract %v3float [[h0]] 1
// CHECK-NEXT: [[g0v1:%[0-9]+]] = OpCompositeExtract %v3float [[g0]] 1
// CHECK-NEXT: [[h1v1:%[0-9]+]] = OpFDiv %v3float [[h0v1]] [[g0v1]]
// CHECK-NEXT: [[h1:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[h1v0]] [[h1v1]]
// CHECK-NEXT: OpStore %h [[h1]]
    h /= g;

    float3x2 i, j;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %mat3v2float %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %mat3v2float %j
// CHECK-NEXT: [[j0v0:%[0-9]+]] = OpCompositeExtract %v2float [[j0]] 0
// CHECK-NEXT: [[i0v0:%[0-9]+]] = OpCompositeExtract %v2float [[i0]] 0
// CHECK-NEXT: [[j1v0:%[0-9]+]] = OpFRem %v2float [[j0v0]] [[i0v0]]
// CHECK-NEXT: [[j0v1:%[0-9]+]] = OpCompositeExtract %v2float [[j0]] 1
// CHECK-NEXT: [[i0v1:%[0-9]+]] = OpCompositeExtract %v2float [[i0]] 1
// CHECK-NEXT: [[j1v1:%[0-9]+]] = OpFRem %v2float [[j0v1]] [[i0v1]]
// CHECK-NEXT: [[j0v2:%[0-9]+]] = OpCompositeExtract %v2float [[j0]] 2
// CHECK-NEXT: [[i0v2:%[0-9]+]] = OpCompositeExtract %v2float [[i0]] 2
// CHECK-NEXT: [[j1v2:%[0-9]+]] = OpFRem %v2float [[j0v2]] [[i0v2]]
// CHECK-NEXT: [[j1:%[0-9]+]] = OpCompositeConstruct %mat3v2float [[j1v0]] [[j1v1]] [[j1v2]]
// CHECK-NEXT: OpStore %j [[j1]]
    j %= i;

// Non-floating point matrices

    int2x3 k, l;
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %k
// CHECK-NEXT: [[l0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %l
// CHECK-NEXT: [[l0v0:%[0-9]+]] = OpCompositeExtract %v3int [[l0]] 0
// CHECK-NEXT: [[k0v0:%[0-9]+]] = OpCompositeExtract %v3int [[k0]] 0
// CHECK-NEXT: [[l1v0:%[0-9]+]] = OpIAdd %v3int [[l0v0]] [[k0v0]]
// CHECK-NEXT: [[l0v1:%[0-9]+]] = OpCompositeExtract %v3int [[l0]] 1
// CHECK-NEXT: [[k0v1:%[0-9]+]] = OpCompositeExtract %v3int [[k0]] 1
// CHECK-NEXT: [[l1v1:%[0-9]+]] = OpIAdd %v3int [[l0v1]] [[k0v1]]
// CHECK-NEXT: [[l1:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[l1v0]] [[l1v1]]
// CHECK-NEXT: OpStore %l [[l1]]
    l += k;

// Note: The front-end disallows using these operators on boolean matrices.
}
