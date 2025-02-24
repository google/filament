// RUN: %dxc -T vs_6_0 -E main -fcgl %s -spirv | FileCheck %s

struct S {
    int3 a;
    uint b;
    float2x2 c;
};

struct T {
    // Same fields as S
    int3 h;
    uint i;
    float2x2 j;

    // Additional field
    bool2 k;

    // Embedded S
    S l;

    // Similar to S but need some casts
    float3 m;
    int n;
    float2x2 o;
};

struct O {
    int x;
};

struct P {
    O y;
    float z;
};

struct Q : O {
  float z;
};

struct W {
  float4 color;
};

struct BitFields {
  uint R:8;
  uint G:8;
  uint B:8;
  uint A:8;
};

struct V
{
    float arr[2];
};

struct U
{
    V val;
};

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // Flat initializer list
// CHECK:      [[a:%[0-9]+]] = OpCompositeConstruct %v3int %int_1 %int_2 %int_3
// CHECK-NEXT: [[c0:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[c1:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT: [[c:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[c0]] [[c1]]
// CHECK-NEXT: [[s1:%[0-9]+]] = OpCompositeConstruct %S [[a]] %uint_42 [[c]]
// CHECK-NEXT: OpStore %s1 [[s1]]
    S s1 = {1, 2, 3, 42, 1., 2., 3., 4.};

    // Random parentheses
// CHECK:      [[a:%[0-9]+]] = OpCompositeConstruct %v3int %int_1 %int_2 %int_3
// CHECK-NEXT: [[c0:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[c1:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT: [[c:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[c0]] [[c1]]
// CHECK-NEXT: [[s2:%[0-9]+]] = OpCompositeConstruct %S [[a]] %uint_42 [[c]]
// CHECK-NEXT: OpStore %s2 [[s2]]
    S s2 = {{1, 2}, 3, {{42}, {{1.}}}, {2., {3., 4.}}};

    // Flat initalizer list for nested structs
// CHECK:      [[y:%[0-9]+]] = OpCompositeConstruct %O %int_1
// CHECK-NEXT: [[p:%[0-9]+]] = OpCompositeConstruct %P [[y]] %float_2
// CHECK-NEXT: OpStore %p [[p]]
    P p = {1, 2.};

    // Initalizer list for struct with inheritance.
// CHECK:      [[y:%[0-9]+]] = OpCompositeConstruct %O %int_1
// CHECK-NEXT: [[q:%[0-9]+]] = OpCompositeConstruct %Q [[y]] %float_2
// CHECK-NEXT: OpStore %q [[q]]
    Q q = {1, 2.};

    // Mixed case: use struct as a whole, decomposing struct, type casting

// CHECK-NEXT: [[s1_val:%[0-9]+]] = OpLoad %S %s1
// CHECK-NEXT: [[l:%[0-9]+]] = OpLoad %S %s2
// CHECK-NEXT: [[s2_val:%[0-9]+]] = OpLoad %S %s2
// CHECK-NEXT: [[h:%[0-9]+]] = OpCompositeExtract %v3int [[s1_val]] 0
// CHECK-NEXT: [[i:%[0-9]+]] = OpCompositeExtract %uint [[s1_val]] 1
// CHECK-NEXT: [[j:%[0-9]+]] = OpCompositeExtract %mat2v2float [[s1_val]] 2

// CHECK-NEXT: [[k:%[0-9]+]] = OpCompositeConstruct %v2bool %true %false

// CHECK-NEXT: [[s2av:%[0-9]+]] = OpCompositeExtract %v3int [[s2_val]] 0
// CHECK-NEXT: [[s2bv:%[0-9]+]] = OpCompositeExtract %uint [[s2_val]] 1
// CHECK-NEXT: [[o:%[0-9]+]] = OpCompositeExtract %mat2v2float [[s2_val]] 2
// CHECK-NEXT: [[m:%[0-9]+]] = OpConvertSToF %v3float [[s2av]]
// CHECK-NEXT: [[n:%[0-9]+]] = OpBitcast %int [[s2bv]]
// CHECK-NEXT: [[t:%[0-9]+]] = OpCompositeConstruct %T [[h]] [[i]] [[j]] [[k]] [[l]] [[m]] [[n]] [[o]]
// CHECK-NEXT: OpStore %t [[t]]
    T t = {s1,          // Decomposing struct
           true, false, // constructing field from scalar
           s2,          // Embedded struct
           s2           // Decomposing struct + type casting
          };

    // Using InitListExpr
// CHECK:        [[int4_zero:%[0-9]+]] = OpCompositeConstruct %v4int %int_0 %int_0 %int_0 %int_0
// CHECK-NEXT: [[float4_zero:%[0-9]+]] = OpConvertSToF %v4float [[int4_zero]]
// CHECK-NEXT:             {{%[0-9]+}} = OpCompositeConstruct %W [[float4_zero]]
    W w = { (0).xxxx };

// CHECK: [[v1:%[0-9]+]] = OpBitFieldInsert %uint %uint_3 %uint_2 %uint_8 %uint_8
// CHECK: [[v2:%[0-9]+]] = OpBitFieldInsert %uint [[v1]] %uint_1 %uint_16 %uint_8
// CHECK: [[v3:%[0-9]+]] = OpBitFieldInsert %uint [[v2]] %uint_0 %uint_24 %uint_8
// CHECK: [[bf:%[0-9]+]] = OpCompositeConstruct %BitFields [[v3]]
// CHECK:                  OpStore %bf [[bf]]
    BitFields bf = {3, 2, 1, 0};

// CHECK:   [[vals:%[0-9]+]] = OpLoad %_ptr_Uniform_type_StructuredBuffer_V %vals
// CHECK: [[vals_0:%[0-9]+]] = OpAccessChain %_ptr_Uniform_V [[vals]] %int_0 %uint_0
// CHECK:      [[V:%[0-9]+]] = OpLoad %V [[vals_0]]
// CHECK:    [[V_0:%[0-9]+]] = OpCompositeExtract %_arr_float_uint_2 [[V]] 0
// CHECK:  [[V_0_0:%[0-9]+]] = OpCompositeExtract %float [[V_0]] 0
// CHECK:  [[V_0_1:%[0-9]+]] = OpCompositeExtract %float [[V_0]] 1
// CHECK:    [[arr:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_2_0 [[V_0_0]] [[V_0_1]]
// CHECK:     [[V2:%[0-9]+]] = OpCompositeConstruct %V_0 [[arr]]
// CHECK:      [[U:%[0-9]+]] = OpCompositeConstruct %U [[V2]]
// CHECK:                      OpStore %u [[U]]
    StructuredBuffer<V> vals;
    U u = { vals[0] };
}
