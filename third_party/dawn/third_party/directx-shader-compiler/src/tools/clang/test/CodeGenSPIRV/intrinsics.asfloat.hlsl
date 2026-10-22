// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'asfloat' function can only operate on int, uint, float,
// vector of these scalars, and matrix of these scalars.

void main() {
    float result;
    float4 result4;
    float1x1 result1x1;
    float1x3 result1x3;
    float2x1 result2x1;
    float2x3 result2x3; 


    // CHECK:      [[a:%[0-9]+]] = OpLoad %int %a
    // CHECK-NEXT: [[a_as_float:%[0-9]+]] = OpBitcast %float [[a]]
    // CHECK-NEXT: OpStore %result [[a_as_float]]
    int a;
    result = asfloat(a);

    // CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %uint %b
    // CHECK-NEXT: [[b_as_float:%[0-9]+]] = OpBitcast %float [[b]]
    // CHECK-NEXT: OpStore %result [[b_as_float]]
    uint b;
    result = asfloat(b);

    // CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %float %c
    // CHECK-NEXT: OpStore %result [[c]]
    float c;
    result = asfloat(c);

    // CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %int %d
    // CHECK-NEXT: [[d_as_float:%[0-9]+]] = OpBitcast %float [[d]]
    // CHECK-NEXT: OpStore %result [[d_as_float]]
    int1 d;
    result = asfloat(d);

    // CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %uint %e
    // CHECK-NEXT: [[e_as_float:%[0-9]+]] = OpBitcast %float [[e]]
    // CHECK-NEXT: OpStore %result [[e_as_float]]
    uint1 e;
    result = asfloat(e);

    // CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %float %f
    // CHECK-NEXT: OpStore %result [[f]]
    float1 f;
    result = asfloat(f);

    // CHECK-NEXT: [[g:%[0-9]+]] = OpLoad %v4int %g
    // CHECK-NEXT: [[g_as_float:%[0-9]+]] = OpBitcast %v4float [[g]]
    // CHECK-NEXT: OpStore %result4 [[g_as_float]]
    int4 g;
    result4 = asfloat(g);

    // CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %v4uint %h
    // CHECK-NEXT: [[h_as_float:%[0-9]+]] = OpBitcast %v4float [[h]]
    // CHECK-NEXT: OpStore %result4 [[h_as_float]]
    uint4 h;
    result4 = asfloat(h);

    // CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v4float %i
    // CHECK-NEXT: OpStore %result4 [[i]]
    float4 i;
    result4 = asfloat(i);
    
    // CHECK-NEXT: [[j:%[0-9]+]] = OpLoad %float %j
    // CHECK-NEXT: OpStore %result1x1 [[j]]
    float1x1 j;
    result1x1 = asfloat(j);
    
    // CHECK-NEXT: [[k:%[0-9]+]] = OpLoad %v3float %k
    // CHECK-NEXT: OpStore %result1x3 [[k]]    
    float1x3 k;
    result1x3 = asfloat(k);
    
    // CHECK-NEXT: [[l:%[0-9]+]] = OpLoad %v2float %l
    // CHECK-NEXT: OpStore %result2x1 [[l]]
    float2x1 l;
    result2x1 = asfloat(l);
    
    // CHECK-NEXT: [[m:%[0-9]+]] = OpLoad %mat2v3float %m
    // CHECK-NEXT: OpStore %result2x3 [[m]]
    float2x3 m;
    result2x3 = asfloat(m);

    int2x3 n;
    uint2x3 o;

// CHECK:           [[n:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %n
// CHECK-NEXT:     [[n0:%[0-9]+]] = OpCompositeExtract %v3int [[n]] 0
// CHECK-NEXT:   [[row0:%[0-9]+]] = OpBitcast %v3float [[n0]]
// CHECK-NEXT:     [[n1:%[0-9]+]] = OpCompositeExtract %v3int [[n]] 1
// CHECK-NEXT:   [[row1:%[0-9]+]] = OpBitcast %v3float [[n1]]
// CHECK-NEXT: [[result:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[row0]] [[row1]]
// CHECK-NEXT:                   OpStore %result2x3 [[result]]
    result2x3 = asfloat(n);
// CHECK:           [[o:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %o
// CHECK-NEXT:     [[o0:%[0-9]+]] = OpCompositeExtract %v3uint [[o]] 0
// CHECK-NEXT:   [[row0_0:%[0-9]+]] = OpBitcast %v3float [[o0]]
// CHECK-NEXT:     [[o1:%[0-9]+]] = OpCompositeExtract %v3uint [[o]] 1
// CHECK-NEXT:   [[row1_0:%[0-9]+]] = OpBitcast %v3float [[o1]]
// CHECK-NEXT: [[result_0:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[row0_0]] [[row1_0]]
// CHECK-NEXT:                   OpStore %result2x3 [[result_0]]
    result2x3 = asfloat(o);
}
