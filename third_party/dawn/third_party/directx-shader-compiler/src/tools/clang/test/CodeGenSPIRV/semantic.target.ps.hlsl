// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpEntryPoint Fragment %main "main" %in_var_A %out_var_SV_Target2 %out_var_SV_Target %out_var_SV_Target1 %out_var_SV_Target3

// CHECK:      OpDecorate %out_var_SV_Target2 Location 2
// CHECK-NEXT: OpDecorate %out_var_SV_Target Location 0
// CHECK-NEXT: OpDecorate %out_var_SV_Target1 Location 1
// CHECK-NEXT: OpDecorate %out_var_SV_Target3 Location 3

// CHECK:      %out_var_SV_Target2 = OpVariable %_ptr_Output_v4float Output
// CHECK-NEXT: %out_var_SV_Target = OpVariable %_ptr_Output_v4float Output
// CHECK-NEXT: %out_var_SV_Target1 = OpVariable %_ptr_Output_v4float Output
// CHECK-NEXT: %out_var_SV_Target3 = OpVariable %_ptr_Output_v4float Output

struct PSOut {
    float4 color2 : SV_Target2;
    float4 color0 : SV_Target;
    float4 color1 : SV_Target1;
    float4 color3 : SV_Target3;
};

PSOut main(float4 input: A) {
    PSOut ret;
    ret.color2 = input;
    ret.color0 = input;
    ret.color1 = input;
    ret.color3 = input;
    return ret;
}
