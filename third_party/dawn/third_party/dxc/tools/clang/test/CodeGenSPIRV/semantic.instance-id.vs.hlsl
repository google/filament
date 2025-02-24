// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:                     OpEntryPoint Vertex %main "main"
// CHECK-SAME:                %gl_InstanceIndex
// CHECK-SAME:                %out_var_SV_InstanceID

// CHECK:                     OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
// CHECK:                     OpDecorate %out_var_SV_InstanceID Location 0

// CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_int Input
// CHECK: %out_var_SV_InstanceID = OpVariable %_ptr_Output_int Output

// CHECK:                     %main = OpFunction
// CHECK: [[gl_InstanceIndex:%[0-9]+]] = OpLoad %int %gl_InstanceIndex
// CHECK:                             OpStore %param_var_input [[gl_InstanceIndex]]
// CHECK:                  {{%[0-9]+}} = OpFunctionCall %int %src_main %param_var_input

int main(int input: SV_InstanceID) : SV_InstanceID {
    return input;
}

