// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float4 m : MMM;
};

struct T {
    float3 n : NNN;
};

struct Base {
    float4 a : AAA;
    float4 b : BBB;
    S      s;
    float4 p : SV_Position;
};

struct Derived : Base {
    T      t;
    float4 c : CCC;
    float4 d : DDD;
};

void main(in Derived input, out Derived output) {
// CHECK:         [[a:%[0-9]+]] = OpLoad %v4float %in_var_AAA
// CHECK-NEXT:    [[b:%[0-9]+]] = OpLoad %v4float %in_var_BBB

// CHECK-NEXT:    [[m:%[0-9]+]] = OpLoad %v4float %in_var_MMM
// CHECK-NEXT:    [[s:%[0-9]+]] = OpCompositeConstruct %S [[m]]

// CHECK-NEXT:  [[pos:%[0-9]+]] = OpLoad %v4float %in_var_SV_Position

// CHECK-NEXT: [[base:%[0-9]+]] = OpCompositeConstruct %Base [[a]] [[b]] [[s]] [[pos]]

// CHECK-NEXT:    [[n:%[0-9]+]] = OpLoad %v3float %in_var_NNN
// CHECK-NEXT:    [[t:%[0-9]+]] = OpCompositeConstruct %T [[n]]

// CHECK-NEXT:    [[c:%[0-9]+]] = OpLoad %v4float %in_var_CCC
// CHECK-NEXT:    [[d:%[0-9]+]] = OpLoad %v4float %in_var_DDD

// CHECK-NEXT:  [[drv:%[0-9]+]] = OpCompositeConstruct %Derived [[base]] [[t]] [[c]] [[d]]
// CHECK-NEXT:                 OpStore %param_var_input [[drv]]

// CHECK-NEXT:      {{%[0-9]+}} = OpFunctionCall %void %src_main %param_var_input %param_var_output

// CHECK-NEXT:  [[drv_0:%[0-9]+]] = OpLoad %Derived %param_var_output

// CHECK-NEXT: [[base_0:%[0-9]+]] = OpCompositeExtract %Base [[drv_0]] 0
// CHECK-NEXT:    [[a_0:%[0-9]+]] = OpCompositeExtract %v4float [[base_0]] 0
// CHECK-NEXT:                 OpStore %out_var_AAA [[a_0]]
// CHECK-NEXT:    [[b_0:%[0-9]+]] = OpCompositeExtract %v4float [[base_0]] 1
// CHECK-NEXT:                 OpStore %out_var_BBB [[b_0]]

// CHECK-NEXT:    [[s_0:%[0-9]+]] = OpCompositeExtract %S [[base_0]] 2
// CHECK-NEXT:    [[m_0:%[0-9]+]] = OpCompositeExtract %v4float [[s_0]] 0
// CHECK-NEXT:                 OpStore %out_var_MMM [[m_0]]

// CHECK-NEXT:  [[pos_0:%[0-9]+]] = OpCompositeExtract %v4float [[base_0]] 3
// CHECK-NEXT:                 OpStore %gl_Position [[pos_0]]

// CHECK-NEXT:    [[t_0:%[0-9]+]] = OpCompositeExtract %T [[drv_0]] 1
// CHECK-NEXT:    [[n_0:%[0-9]+]] = OpCompositeExtract %v3float [[t_0]] 0
// CHECK-NEXT:                 OpStore %out_var_NNN [[n_0]]

// CHECK-NEXT:    [[c_0:%[0-9]+]] = OpCompositeExtract %v4float [[drv_0]] 2
// CHECK-NEXT:                 OpStore %out_var_CCC [[c_0]]
// CHECK-NEXT:    [[d_0:%[0-9]+]] = OpCompositeExtract %v4float [[drv_0]] 3
// CHECK-NEXT:                 OpStore %out_var_DDD [[d_0]]
    output = input;
}
