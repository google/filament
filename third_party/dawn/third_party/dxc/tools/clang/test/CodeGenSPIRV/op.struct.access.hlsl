// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    bool     a;
    uint2    b;
    float2x3 c;
    float4   d;
    float4   e[1];
    float    f[4];
    int      g;
};

struct T {
    int h; // Nested struct
    S i;
};

T foo() {
    T ret = (T)0;
    return ret;
}

S bar() {
    S ret = (S)0;
    return ret;
}

ConstantBuffer<S> MyBuffer;

S baz() {
    return MyBuffer;
}

float4 main() : SV_Target {
    T t;

// CHECK:      [[h:%[0-9]+]] = OpAccessChain %_ptr_Function_int %t %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %int [[h]]
    int v1 = t.h;
// CHECK:      [[a:%[0-9]+]] = OpAccessChain %_ptr_Function_bool %t %int_1 %int_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %bool [[a]]
    bool v2 = t.i.a;

// CHECK:      [[b0:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %t %int_1 %int_1 %uint_0
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %uint [[b0]]
    uint v3 = t.i.b[0];
// CHECK:      [[b:%[0-9]+]] = OpAccessChain %_ptr_Function_v2uint %t %int_1 %int_1
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %v2uint [[b]]
    uint2 v4 = t.i.b.rg;

// CHECK:      [[c:%[0-9]+]] = OpAccessChain %_ptr_Function_mat2v3float %t %int_1 %int_2
// CHECK-NEXT: [[c00p:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[c]] %int_0 %int_0
// CHECK-NEXT: [[c00v:%[0-9]+]] = OpLoad %float [[c00p]]
// CHECK-NEXT: [[c11p:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[c]] %int_1 %int_1
// CHECK-NEXT: [[c11v:%[0-9]+]] = OpLoad %float [[c11p]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %v2float [[c00v]] [[c11v]]
    float2 v5 = t.i.c._11_22;
// CHECK:      [[c1:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %t %int_1 %int_2 %uint_1
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %v3float [[c1]]
    float3 v6 = t.i.c[1];

// CHECK:      [[h_0:%[0-9]+]] = OpAccessChain %_ptr_Function_int %t %int_0
// CHECK-NEXT: OpStore [[h_0]] {{%[0-9]+}}
    t.h = v1;
// CHECK:      [[a_0:%[0-9]+]] = OpAccessChain %_ptr_Function_bool %t %int_1 %int_0
// CHECK-NEXT: OpStore [[a_0]] {{%[0-9]+}}
    t.i.a = v2;

// CHECK:      [[b1:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %t %int_1 %int_1 %uint_1
// CHECK-NEXT: OpStore [[b1]] {{%[0-9]+}}
    t.i.b[1] = v3;
// CHECK:      [[v4:%[0-9]+]] = OpLoad %v2uint %v4
// CHECK-NEXT: [[b_0:%[0-9]+]] = OpAccessChain %_ptr_Function_v2uint %t %int_1 %int_1
// CHECK-NEXT: [[bv:%[0-9]+]] = OpLoad %v2uint [[b_0]]
// CHECK-NEXT: [[gr:%[0-9]+]] = OpVectorShuffle %v2uint [[bv]] [[v4]] 3 2
// CHECK-NEXT: OpStore [[b_0]] [[gr]]
    t.i.b.gr = v4;

// CHECK:      [[v5:%[0-9]+]] = OpLoad %v2float %v5
// CHECK-NEXT: [[c_0:%[0-9]+]] = OpAccessChain %_ptr_Function_mat2v3float %t %int_1 %int_2
// CHECK-NEXT: [[v50:%[0-9]+]] = OpCompositeExtract %float [[v5]] 0
// CHECK-NEXT: [[c11:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[c_0]] %int_1 %int_1
// CHECK-NEXT: OpStore [[c11]] [[v50]]
// CHECK-NEXT: [[v51:%[0-9]+]] = OpCompositeExtract %float [[v5]] 1
// CHECK-NEXT: [[c00:%[0-9]+]] = OpAccessChain %_ptr_Function_float [[c_0]] %int_0 %int_0
// CHECK-NEXT: OpStore [[c00]] [[v51]]
    t.i.c._22_11 = v5;
// CHECK:      [[c0:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %t %int_1 %int_2 %uint_0
// CHECK-NEXT: OpStore [[c0]] {{%[0-9]+}}
    t.i.c[0] = v6;

// CHECK:       [[baz:%[0-9]+]] = OpFunctionCall %S %baz
// CHECK-NEXT:                 OpStore %temp_var_S [[baz]]
// CHECK-NEXT: [[base:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_v4float_uint_1 %temp_var_S %int_4
// CHECK-NEXT: [[array_value:%[0-9]+]] = OpLoad %_arr_v4float_uint_1 [[base]]
// CHECK-NEXT:                 OpStore %temp_var_ [[array_value]]
// CHECK-NEXT:                 OpAccessChain %_ptr_Function_v4float %temp_var_ %int_0
// CHECK:       [[bar:%[0-9]+]] = OpFunctionCall %S %bar
// CHECK-NEXT:                 OpStore %temp_var_S_0 [[bar]]
// CHECK-NEXT: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function__arr_float_uint_4 %temp_var_S_0 %int_5
// CHECK-NEXT: [[ld:%[0-9]+]] = OpLoad %_arr_float_uint_4 %111
// CHECK-NEXT:                 OpStore %temp_var__0 %112
// CHECK-NEXT:                 OpAccessChain %_ptr_Function_float %temp_var__0 %int_1
    float4 val1 = bar().f[1] * baz().e[0];

// CHECK:        [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function_int %temp_var_S_1 %int_6
// CHECK-NEXT:                 OpLoad %int [[ac]]
    bool val2 = bar().g; // Need cast on rvalue function return

// CHECK:      [[ret:%[0-9]+]] = OpFunctionCall %T %foo
// CHECK-NEXT: OpStore %temp_var_T [[ret]]
// CHECK-NEXT: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Function_v4float %temp_var_T %int_1 %int_3
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %v4float [[ptr]]
// CHECK-NEXT: OpReturnValue [[val]]
    return foo().i.d;
}
