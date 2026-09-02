// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'any' function can only operate on int, bool, float,
// vector of these scalars, and matrix of these scalars.

// CHECK:   [[v4int_0:%[0-9]+]] = OpConstantComposite %v4int %int_0 %int_0 %int_0 %int_0
// CHECK:  [[v4uint_0:%[0-9]+]] = OpConstantComposite %v4uint %uint_0 %uint_0 %uint_0 %uint_0
// CHECK: [[v4float_0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[v3float_0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0
// CHECK: [[v2float_0:%[0-9]+]] = OpConstantComposite %v2float %float_0 %float_0

void main() {
    bool result;

    // CHECK:      [[a:%[0-9]+]] = OpLoad %int %a
    // CHECK-NEXT: [[any_int:%[0-9]+]] = OpINotEqual %bool [[a]] %int_0
    // CHECK-NEXT: OpStore %result [[any_int]]
    int a;
    result = any(a);

    // CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %uint %b
    // CHECK-NEXT: [[any_uint:%[0-9]+]] = OpINotEqual %bool [[b]] %uint_0
    // CHECK-NEXT: OpStore %result [[any_uint]]
    uint b;
    result = any(b);

    // CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %bool %c
    // CHECK-NEXT: OpStore %result [[c]]
    bool c;
    result = any(c);

    // CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %float %d
    // CHECK-NEXT: [[any_float:%[0-9]+]] = OpFOrdNotEqual %bool [[d]] %float_0
    // CHECK-NEXT: OpStore %result [[any_float]]
    float d;
    result = any(d);

    // CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %int %e
    // CHECK-NEXT: [[any_int1:%[0-9]+]] = OpINotEqual %bool [[e]] %int_0
    // CHECK-NEXT: OpStore %result [[any_int1]]
    int1 e;
    result = any(e);

    // CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %uint %f
    // CHECK-NEXT: [[any_uint1:%[0-9]+]] = OpINotEqual %bool [[f]] %uint_0
    // CHECK-NEXT: OpStore %result [[any_uint1]]
    uint1 f;
    result = any(f);

    // CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %bool %g
    // CHECK-NEXT: OpStore %result [[g]]
    bool1 g;
    result = any(g);

    // CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %float %h
    // CHECK-NEXT: [[any_float1:%[0-9]+]] = OpFOrdNotEqual %bool [[h]] %float_0
    // CHECK-NEXT: OpStore %result [[any_float1]]
    float1 h;
    result = any(h);

    // CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v4int %i
    // CHECK-NEXT: [[v4int_to_bool:%[0-9]+]] = OpINotEqual %v4bool [[i]] [[v4int_0]]
    // CHECK-NEXT: [[any_int4:%[0-9]+]] = OpAny %bool [[v4int_to_bool]]
    // CHECK-NEXT: OpStore %result [[any_int4]]
    int4 i;
    result = any(i);

    // CHECK-NEXT: [[j:%[0-9]+]] = OpLoad %v4uint %j
    // CHECK-NEXT: [[v4uint_to_bool:%[0-9]+]] = OpINotEqual %v4bool [[j]] [[v4uint_0]]
    // CHECK-NEXT: [[any_uint4:%[0-9]+]] = OpAny %bool [[v4uint_to_bool]]
    // CHECK-NEXT: OpStore %result [[any_uint4]]
    uint4 j;
    result = any(j);

    // CHECK-NEXT: [[k:%[0-9]+]] = OpLoad %v4bool %k
    // CHECK-NEXT: [[any_bool4:%[0-9]+]] = OpAny %bool [[k]]
    // CHECK-NEXT: OpStore %result [[any_bool4]]
    bool4 k;
    result = any(k);

    // CHECK-NEXT: [[l:%[0-9]+]] = OpLoad %v4float %l
    // CHECK-NEXT: [[v4float_to_bool:%[0-9]+]] = OpFOrdNotEqual %v4bool [[l]] [[v4float_0]]
    // CHECK-NEXT: [[any_float4:%[0-9]+]] = OpAny %bool [[v4float_to_bool]]
    // CHECK-NEXT: OpStore %result [[any_float4]]
    float4 l;
    result = any(l);

    // CHECK-NEXT: [[m:%[0-9]+]] = OpLoad %float %m
    // CHECK-NEXT: [[mat1x1_to_bool:%[0-9]+]] = OpFOrdNotEqual %bool [[m]] %float_0
    // CHECK-NEXT: OpStore %result [[mat1x1_to_bool]]
    float1x1 m;
    result = any(m);

    // CHECK-NEXT: [[n:%[0-9]+]] = OpLoad %v3float %n
    // CHECK-NEXT: [[mat1x3_to_bool:%[0-9]+]] = OpFOrdNotEqual %v3bool [[n]] [[v3float_0]]
    // CHECK-NEXT: [[any_mat1x3:%[0-9]+]] = OpAny %bool [[mat1x3_to_bool]]
    // CHECK-NEXT: OpStore %result [[any_mat1x3]]
    float1x3 n;
    result = any(n);

    // CHECK-NEXT: [[o:%[0-9]+]] = OpLoad %v2float %o
    // CHECK-NEXT: [[mat2x1_to_bool:%[0-9]+]] = OpFOrdNotEqual %v2bool [[o]] [[v2float_0]]
    // CHECK-NEXT: [[any_mat2x1:%[0-9]+]] = OpAny %bool [[mat2x1_to_bool]]
    // CHECK-NEXT: OpStore %result [[any_mat2x1]]
    float2x1 o;
    result = any(o);

    // CHECK-NEXT: [[p:%[0-9]+]] = OpLoad %mat3v4float %p
    // CHECK-NEXT: [[row0:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 0
    // CHECK-NEXT: [[row0_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row0]] [[v4float_0]]
    // CHECK-NEXT: [[any_row0:%[0-9]+]] = OpAny %bool [[row0_to_bool_vec]]
    // CHECK-NEXT: [[row1:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 1
    // CHECK-NEXT: [[row1_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row1]] [[v4float_0]]
    // CHECK-NEXT: [[any_row1:%[0-9]+]] = OpAny %bool [[row1_to_bool_vec]]
    // CHECK-NEXT: [[row2:%[0-9]+]] = OpCompositeExtract %v4float [[p]] 2
    // CHECK-NEXT: [[row2_to_bool_vec:%[0-9]+]] = OpFOrdNotEqual %v4bool [[row2]] [[v4float_0]]
    // CHECK-NEXT: [[any_row2:%[0-9]+]] = OpAny %bool [[row2_to_bool_vec]]
    // CHECK-NEXT: [[any_rows:%[0-9]+]] = OpCompositeConstruct %v3bool [[any_row0]] [[any_row1]] [[any_row2]]
    // CHECK-NEXT: [[any_mat3x4:%[0-9]+]] = OpAny %bool [[any_rows]]
    // CHECK-NEXT: OpStore %result [[any_mat3x4]]
    float3x4 p;
    result = any(p);
}

