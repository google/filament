// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2f1:%[0-9]+]] = OpConstantComposite %v2float %float_1 %float_1
// CHECK: [[v3f1:%[0-9]+]] = OpConstantComposite %v3float %float_1 %float_1 %float_1
// CHECK: [[v3i1:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_1 %int_1
void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // 1x1
    float1x1 a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[a1:%[0-9]+]] = OpFAdd %float [[a0]] %float_1
// CHECK-NEXT: OpStore %a [[a1]]
// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: OpStore %b [[a2]]
    b = ++a;
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %float %b
// CHECK-NEXT: [[a3:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT: [[a4:%[0-9]+]] = OpFAdd %float [[a3]] %float_1
// CHECK-NEXT: OpStore %a [[a4]]
// CHECK-NEXT: OpStore %a [[b0]]
    ++a = b;

    // Mx1
    float2x1 c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %v2float %c
// CHECK-NEXT: [[c1:%[0-9]+]] = OpFAdd %v2float [[c0]] [[v2f1]]
// CHECK-NEXT: OpStore %c [[c1]]
// CHECK-NEXT: [[c2:%[0-9]+]] = OpLoad %v2float %c
// CHECK-NEXT: OpStore %d [[c2]]
    d = ++c;
// CHECK-NEXT: [[d0:%[0-9]+]] = OpLoad %v2float %d
// CHECK-NEXT: [[c3:%[0-9]+]] = OpLoad %v2float %c
// CHECK-NEXT: [[c4:%[0-9]+]] = OpFAdd %v2float [[c3]] [[v2f1]]
// CHECK-NEXT: OpStore %c [[c4]]
// CHECK-NEXT: OpStore %c [[d0]]
    ++c = d;

    // 1xN
    float1x3 e, f;
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %v3float %e
// CHECK-NEXT: [[e1:%[0-9]+]] = OpFAdd %v3float [[e0]] [[v3f1]]
// CHECK-NEXT: OpStore %e [[e1]]
// CHECK-NEXT: [[e2:%[0-9]+]] = OpLoad %v3float %e
// CHECK-NEXT: OpStore %f [[e2]]
    f = ++e;
// CHECK-NEXT: [[f0:%[0-9]+]] = OpLoad %v3float %f
// CHECK-NEXT: [[e3:%[0-9]+]] = OpLoad %v3float %e
// CHECK-NEXT: [[e4:%[0-9]+]] = OpFAdd %v3float [[e3]] [[v3f1]]
// CHECK-NEXT: OpStore %e [[e4]]
// CHECK-NEXT: OpStore %e [[f0]]
    ++e = f;

    // MxN
    float2x3 g, h;
// CHECK-NEXT: [[g0:%[0-9]+]] = OpLoad %mat2v3float %g
// CHECK-NEXT: [[g0v0:%[0-9]+]] = OpCompositeExtract %v3float [[g0]] 0
// CHECK-NEXT: [[inc0:%[0-9]+]] = OpFAdd %v3float [[g0v0]] [[v3f1]]
// CHECK-NEXT: [[g0v1:%[0-9]+]] = OpCompositeExtract %v3float [[g0]] 1
// CHECK-NEXT: [[inc1:%[0-9]+]] = OpFAdd %v3float [[g0v1]] [[v3f1]]
// CHECK-NEXT: [[g1:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[inc0]] [[inc1]]
// CHECK-NEXT: OpStore %g [[g1]]
// CHECK-NEXT: [[g2:%[0-9]+]] = OpLoad %mat2v3float %g
// CHECK-NEXT: OpStore %h [[g2]]
    h = ++g;
// CHECK-NEXT: [[h0:%[0-9]+]] = OpLoad %mat2v3float %h
// CHECK-NEXT: [[g3:%[0-9]+]] = OpLoad %mat2v3float %g
// CHECK-NEXT: [[g3v0:%[0-9]+]] = OpCompositeExtract %v3float [[g3]] 0
// CHECK-NEXT: [[inc2:%[0-9]+]] = OpFAdd %v3float [[g3v0]] [[v3f1]]
// CHECK-NEXT: [[g3v1:%[0-9]+]] = OpCompositeExtract %v3float [[g3]] 1
// CHECK-NEXT: [[inc3:%[0-9]+]] = OpFAdd %v3float [[g3v1]] [[v3f1]]
// CHECK-NEXT: [[g4:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[inc2]] [[inc3]]
// CHECK-NEXT: OpStore %g [[g4]]
// CHECK-NEXT: OpStore %g [[h0]]
    ++g = h;

    int2x3 m, n;
// CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %m
// CHECK-NEXT: [[m0v0:%[0-9]+]] = OpCompositeExtract %v3int [[m0]] 0
// CHECK-NEXT: [[inc0_0:%[0-9]+]] = OpIAdd %v3int [[m0v0]] [[v3i1]]
// CHECK-NEXT: [[m0v1:%[0-9]+]] = OpCompositeExtract %v3int [[m0]] 1
// CHECK-NEXT: [[inc1_0:%[0-9]+]] = OpIAdd %v3int [[m0v1]] [[v3i1]]
// CHECK-NEXT: [[m1:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[inc0_0]] [[inc1_0]]
// CHECK-NEXT: OpStore %m [[m1]]
// CHECK-NEXT: [[m2:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %m
// CHECK-NEXT: OpStore %n [[m2]]
    n = ++m;
// CHECK-NEXT: [[n0:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %n
// CHECK-NEXT: [[m3:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %m
// CHECK-NEXT: [[m3v0:%[0-9]+]] = OpCompositeExtract %v3int [[m3]] 0
// CHECK-NEXT: [[inc2_0:%[0-9]+]] = OpIAdd %v3int [[m3v0]] [[v3i1]]
// CHECK-NEXT: [[m3v1:%[0-9]+]] = OpCompositeExtract %v3int [[m3]] 1
// CHECK-NEXT: [[inc3_0:%[0-9]+]] = OpIAdd %v3int [[m3v1]] [[v3i1]]
// CHECK-NEXT: [[m4:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[inc2_0]] [[inc3_0]]
// CHECK-NEXT: OpStore %m [[m4]]
// CHECK-NEXT: OpStore %m [[n0]]
    ++m = n;

// Note: Boolean matrices are not allowed by the front-end.
}
