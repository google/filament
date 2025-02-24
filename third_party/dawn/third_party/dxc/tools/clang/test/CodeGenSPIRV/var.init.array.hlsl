// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S1 {
    float2 a;
};

struct S2 {
    float2 b[2];
};

struct T1 {
    S2 c;  // Need to split to match T2.f1 & T2.f2
    S2 d;  // Match T2.f3 exactly
};

struct T2 {
    S1 e;
    S1 f;
    S2 g;
};

// Flattend T2: need to split all fields in T2
struct T3 {
    float2 h;
    float2 i;
    float2 j;
    float2 k;
};

void main() {
    T1 val1[2];

// CHECK:          [[val1:%[0-9]+]] = OpLoad %_arr_T1_uint_2 %val1
// CHECK-NEXT:   [[val1_0:%[0-9]+]] = OpCompositeExtract %T1 [[val1]] 0
// CHECK-NEXT:   [[val1_1:%[0-9]+]] = OpCompositeExtract %T1 [[val1]] 1
// CHECK-NEXT: [[val1_2:%[0-9]+]] = OpCompositeExtract %S2 [[val1_0]] 0
// CHECK-NEXT: [[val1_0_1:%[0-9]+]] = OpCompositeExtract %S2 [[val1_0]] 1
// CHECK-NEXT:      [[t1c:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_2 [[val1_2]] 0
// CHECK-NEXT:    [[t1c_0:%[0-9]+]] = OpCompositeExtract %v2float [[t1c]] 0
// CHECK-NEXT:    [[t1c_1:%[0-9]+]] = OpCompositeExtract %v2float [[t1c]] 1

// val2[0]: Construct T2.e from T1.c.b[0]
// CHECK-NEXT:     [[T2_e:%[0-9]+]] = OpCompositeConstruct %S1 [[t1c_0]]

// val2[0]: Construct T2.f from T1.c.b[1]
// CHECK-NEXT:     [[T2_f:%[0-9]+]] = OpCompositeConstruct %S1 [[t1c_1]]

// val2[0]: Construct val2[0]
// CHECK-NEXT:     [[T2_0:%[0-9]+]] = OpCompositeConstruct %T2 [[T2_e]] [[T2_f]] [[val1_0_1]]

// CHECK-NEXT: [[val1_1_0:%[0-9]+]] = OpCompositeExtract %S2 [[val1_1]] 0
// CHECK-NEXT: [[val1_1_1:%[0-9]+]] = OpCompositeExtract %S2 [[val1_1]] 1
// CHECK-NEXT:      [[t1d:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_2 [[val1_1_0]] 0
// CHECK-NEXT:    [[t1d_0:%[0-9]+]] = OpCompositeExtract %v2float [[t1d]] 0
// CHECK-NEXT:    [[t1d_1:%[0-9]+]] = OpCompositeExtract %v2float [[t1d]] 1

// val2[1]: Construct T2.e from T1.c.b[0]
// CHECK-NEXT:     [[T2_e_0:%[0-9]+]] = OpCompositeConstruct %S1 [[t1d_0]]

// val2[1]: Construct T2.f from T1.c.b[1]
// CHECK-NEXT:     [[T2_f_0:%[0-9]+]] = OpCompositeConstruct %S1 [[t1d_1]]

// val2[1]: Construct val2[1]
// CHECK-NEXT:     [[T2_1:%[0-9]+]] = OpCompositeConstruct %T2 [[T2_e_0]] [[T2_f_0]] [[val1_1_1]]

// CHECK-NEXT:     [[val2:%[0-9]+]] = OpCompositeConstruct %_arr_T2_uint_2 [[T2_0]] [[T2_1]]
// CHECK-NEXT:                     OpStore %val2 [[val2]]
    T2 val2[2] = {val1};

// CHECK:          [[val1_0:%[0-9]+]] = OpAccessChain %_ptr_Function_T1 %val1 %int_0
// CHECK-NEXT:       [[t1:%[0-9]+]] = OpLoad %T1 [[val1_0]]

// val3[1]
// CHECK-NEXT:       [[t3:%[0-9]+]] = OpLoad %T3 %t3

// CHECK-NEXT:     [[s1_0:%[0-9]+]] = OpLoad %S1 %s1
// CHECK-NEXT:       [[s2:%[0-9]+]] = OpLoad %S2 %s2
// CHECK-NEXT:     [[s1_1:%[0-9]+]] = OpLoad %S1 %s1

// val3[0]: Construct T3.h from T1.c.b[0]
// CHECK-NEXT:      [[t1c_0:%[0-9]+]] = OpCompositeExtract %S2 [[t1]] 0
// CHECK-NEXT:      [[t1d_0:%[0-9]+]] = OpCompositeExtract %S2 [[t1]] 1
// CHECK-NEXT:    [[t1c_1:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_2 [[t1c_0]] 0
// CHECK-NEXT:    [[v2f_0:%[0-9]+]] = OpCompositeExtract %v2float [[t1c_1]] 0

// val3[0]: Construct T3.i from T1.c.b[1]
// CHECK-NEXT:    [[v2f_1:%[0-9]+]] = OpCompositeExtract %v2float [[t1c_1]] 1

// val3[0]: Construct T3.j from T1.d.b[0]
// CHECK-NEXT:    [[t1c_1_0:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_2 [[t1d_0]] 0
// CHECK-NEXT:    [[v2f_2:%[0-9]+]] = OpCompositeExtract %v2float [[t1c_1_0]] 0

// val3[0]: Construct T3.k from T1.d.b[1]
// CHECK-NEXT:    [[v2f_3:%[0-9]+]] = OpCompositeExtract %v2float [[t1c_1_0]] 1

// CHECK-NEXT:   [[val3_0:%[0-9]+]] = OpCompositeConstruct %T3 [[v2f_0]] [[v2f_1]] [[v2f_2]] [[v2f_3]]

// val3[2]: Construct T3.h from S1.a
// CHECK-NEXT:     [[t3_h:%[0-9]+]] = OpCompositeExtract %v2float [[s1_0]] 0

// val3[2]: Construct T3.i from S2.b[0]
// CHECK-NEXT:     [[s2_0:%[0-9]+]] = OpCompositeExtract %_arr_v2float_uint_2 [[s2]] 0
// CHECK-NEXT:     [[t3_i:%[0-9]+]] = OpCompositeExtract %v2float [[s2_0]] 0

// val3[2]: Construct T3.j from S2.b[1]
// CHECK-NEXT:     [[t3_j:%[0-9]+]] = OpCompositeExtract %v2float [[s2_0]] 1

// val3[2]: Construct T3.k from S1.a
// CHECK-NEXT:     [[t3_k:%[0-9]+]] = OpCompositeExtract %v2float [[s1_1]] 0

// CHECK-NEXT:   [[val3_2:%[0-9]+]] = OpCompositeConstruct %T3 [[t3_h]] [[t3_i]] [[t3_j]] [[t3_k]]

// CHECK-NEXT:     [[val3:%[0-9]+]] = OpCompositeConstruct %_arr_T3_uint_3 [[val3_0]] [[t3]] [[val3_2]]
// CHECK-NEXT:                     OpStore %val3 [[val3]]
    S1 s1;
    S2 s2;
    T3 t3;
    T3 val3[3] = {val1[0],
                  t3,
                  s1, s2, s1};
}
