// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// %S  : generated from struct S, with    layout decorations
// %S_0: generated from struct S, without layout decorations

// CHECK: %type_MyCBuffer = OpTypeStruct %v4float
cbuffer MyCBuffer {
    float4 cb_val;

    float4 get_cb_val() { return cb_val; }
}

struct S {
    float3 s_val;

    float3 get_s_val() { return s_val; }
};

// CHECK: %type_MyTBuffer = OpTypeStruct %float %S
tbuffer MyTBuffer {
    float tb_val;
    S     tb_s;

    float get_tb_val() { return tb_val; }
}

float4 main() : SV_Target {
// %S vs %S_0: need destruction and construction
// CHECK:       [[tb_s:%[0-9]+]] = OpAccessChain %_ptr_Uniform_S %MyTBuffer %int_1
// CHECK-NEXT:       {{%[0-9]+}} = OpFunctionCall %v3float %S_get_s_val [[tb_s]]
    return get_cb_val() + float4(tb_s.get_s_val(), 0.) * get_tb_val();
}

// CHECK:      %get_cb_val = OpFunction %v4float None {{%[0-9]+}}
// CHECK:         {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_v4float %MyCBuffer %int_0

// CHECK:     %S_get_s_val = OpFunction %v3float None {{%[0-9]+}}
// CHECK-NEXT: %param_this = OpFunctionParameter %_ptr_Function_S_0
// CHECK:         {{%[0-9]+}} = OpAccessChain %_ptr_Function_v3float %param_this %int_0

// CHECK:      %get_tb_val = OpFunction %float None {{%[0-9]+}}
// CHECK:         {{%[0-9]+}} = OpAccessChain %_ptr_Uniform_float %MyTBuffer %int_0
