// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'all' function can only operate on int, bool, float,
// vector of these scalars, and matrix of these scalars.

// CHECK:   [[v4int_0:%[0-9]+]] = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
// CHECK:  [[v4uint_0:%[0-9]+]] = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_0
// CHECK: [[v4float_0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[v3float_0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: [[v2float_0:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_0
// CHECK:   [[v3int_0:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0

void main() {
    bool result;

    // CHECK:      [[a:%[0-9]+]] = OpLoad %int %a
    // CHECK-NEXT: [[all_int:%[0-9]+]] = OpINotEqual %bool [[a]] %int_0
    // CHECK-NEXT: OpStore %result [[all_int]]
    int a;
    result = all(a);

    // CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %uint %b
    // CHECK-NEXT: [[all_uint:%[0-9]+]] = OpINotEqual %bool [[b]] %uint_0
    // CHECK-NEXT: OpStore %result [[all_uint]]
    uint b;
    result = all(b);

    // CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %bool %c
    // CHECK-NEXT: OpStore %result [[c]]
    bool c;
    result = all(c);

    // CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
    // CHECK-NEXT: [[all_float:%[0-9]+]] = OpFOrdNotEqual %bool [[d]] %float_0
    // CHECK-NEXT: OpStore %result [[all_float]]
    float d;
    result = all(d);

    // CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %int %e
    // CHECK-NEXT: [[all_int1:%[0-9]+]] = OpINotEqual %bool [[e]] %int_0
    // CHECK-NEXT: OpStore %result [[all_int1]]
    int1 e;
    result = all(e);

    // CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %uint %f
    // CHECK-NEXT: [[all_uint1:%[0-9]+]] = OpINotEqual %bool [[f]] %uint_0
    // CHECK-NEXT: OpStore %result [[all_uint1]]
    uint1 f;
    result = all(f);

    // CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %bool %g
    // CHECK-NEXT: OpStore %result [[g]]
    bool1 g;
    result = all(g);

    // CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %float %h
    // CHECK-NEXT: [[all_float1:%[0-9]+]] = OpFOrdNotEqual %bool [[h]] %float_0
    // CHECK-NEXT: OpStore %result [[all_float1]]
    float1 h;
    result = all(h);

    // CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v4int %i
    // CHECK-NEXT: [[v4int_to_bool:%[0-9]+]] = OpINotEqual %v4bool [[i]] [[v4int_0]]
    // CHECK-NEXT: [[all_int4:%[0-9]+]] = OpAll %bool [[v4int_to_bool]]
    // CHECK-NEXT: OpStore %result [[all_int4]]
    int4 i;
    result = all(i);

    // CHECK-NEXT: [[j:%[0-9]+]] = OpLoad %v4uint %j
    // CHECK-NEXT: [[v4uint_to_bool:%[0-9]+]] = OpINotEqual %v4bool [[j]] [[v4uint_0]]
    // CHECK-NEXT: [[all_uint4:%[0-9]+]] = OpAll %bool [[v4uint_to_bool]]
    // CHECK-NEXT: OpStore %result [[all_uint4]]
    uint4 j;
    result = all(j);

    // CHECK-NEXT: [[k:%[0-9]+]] = OpLoad %v4bool %k
    // CHECK-NEXT: [[all_bool4:%[0-9]+]] = OpAll %bool [[k]]
    // CHECK-NEXT: OpStore %result [[all_bool4]]
    bool4 k;
    result = all(k);

    // CHECK-NEXT: [[l:%[0-9]+]] = OpLoad %v4float %l
    // CHECK-NEXT: [[v4float_to_bool:%[0-9]+]] = OpFOrdNotEqual %v4bool [[l]] [[v4float_0]]
    // CHECK-NEXT: [[all_float4:%[0-9]+]] = OpAll %bool [[v4float_to_bool]]
    // CHECK-NEXT: OpStore %result [[all_float4]]
    float4 l;
    result = all(l);

    // CHECK-NEXT: [[m:%[0-9]+]] = OpLoad %float %m
    // CHECK-NEXT: [[mat1x1_to_bool:%[0-9]+]] = OpFOrdNotEqual %bool [[m]] %float_0
    // CHECK-NEXT: OpStore %result [[mat1x1_to_bool]]
    float1x1 m;
    result = all(m);

    // CHECK-NEXT: [[n:%[0-9]+]] = OpLoad %v3float %n
    // CHECK-NEXT: [[mat1x3_to_bool:%[0-9]+]] = OpFOrdNotEqual %v3bool [[n]] [[v3float_0]]
    // CHECK-NEXT: [[all_mat1x3:%[0-9]+]] = OpAll %bool [[mat1x3_to_bool]]
    // CHECK-NEXT: OpStore %result [[all_mat1x3]]
    float1x3 n;
    result = all(n);

    // CHECK-NEXT: [[o:%[0-9]+]] = OpLoad %v2float %o
    // CHECK-NEXT: [[mat2x1_to_bool:%[0-9]+]] = OpFOrdNotEqual %v2bool [[o]] [[v2float_0]]
    // CHECK-NEXT: [[all_mat2x1:%[0-9]+]] = OpAll %bool [[mat2x1_to_bool]]
    // CHECK-NEXT: OpStore %result [[all_mat2x1]]
    float2x1 o;
    result = all(o);

    // CHECK-NEXT: [[p:%[0-9]+]] = OpLoad %mat3v4float %p
    // CHECK-NEXT: [[row0:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 0
    // CHECK-NEXT: [[row0_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row0]] [[v4float_0]]
    // CHECK-NEXT: [[all_row0:%[0-9]+]] = OpAll %bool [[row0_to_bool_vec]]
    // CHECK-NEXT: [[row1:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 1
    // CHECK-NEXT: [[row1_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row1]] [[v4float_0]]
    // CHECK-NEXT: [[all_row1:%[0-9]+]] = OpAll %bool [[row1_to_bool_vec]]
    // CHECK-NEXT: [[row2:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 2
    // CHECK-NEXT: [[row2_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row2]] [[v4float_0]]
    // CHECK-NEXT: [[all_row2:%[0-9]+]] = OpAll %bool [[row2_to_bool_vec]]
    // CHECK-NEXT: [[all_rows:%[0-9]+]] = OpCompositeConstruct %v3bool [[all_row0]] [[all_row1]] [[all_row2]]
    // CHECK-NEXT: [[all_mat3x4:%[0-9]+]] = OpAll %bool [[all_rows]]
    // CHECK-NEXT: OpStore %result [[all_mat3x4]]
    float3x4 p;
    result = all(p);

// CHECK:              [[q:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %q
// CHECK-NEXT:      [[row0_0:%[0-9]+]] = OpCompositeExtract %v3int [[q]] 0
// CHECK-NEXT: [[row0_bool:%[0-9]+]] = OpINotEqual %v3bool [[row0_0]] [[v3int_0]]
// CHECK-NEXT:  [[row0_all:%[0-9]+]] = OpAll %bool [[row0_bool]]
// CHECK-NEXT:      [[row1_0:%[0-9]+]] = OpCompositeExtract %v3int [[q]] 1
// CHECK-NEXT: [[row1_bool:%[0-9]+]] = OpINotEqual %v3bool [[row1_0]] [[v3int_0]]
// CHECK-NEXT:  [[row1_all:%[0-9]+]] = OpAll %bool [[row1_bool]]
// CHECK-NEXT:  [[all_rows_0:%[0-9]+]] = OpCompositeConstruct %v2bool [[row0_all]] [[row1_all]]
// CHECK-NEXT:           {{%[0-9]+}} = OpAll %bool [[all_rows_0]]
    int2x3 q;
    result = all(q);
}
