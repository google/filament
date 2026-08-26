// RUN: %dxc -T vs_6_0 -E main -fvk-support-nonzero-base-instance -fcgl  %s -spirv | FileCheck %s

// CHECK:                     OpEntryPoint Vertex %main "main"
// CHECK-SAME:                %gl_InstanceIndex
// CHECK-SAME:                %gl_BaseInstance
// CHECK-SAME:                %out_var_SV_InstanceID

// CHECK:                     OpDecorate %gl_InstanceIndex BuiltIn InstanceIndex
// CHECK:                     OpDecorate %gl_BaseInstance BuiltIn BaseInstance
// CHECK:                     OpDecorate %out_var_SV_InstanceID Location 0

// CHECK: %gl_InstanceIndex = OpVariable %_ptr_Input_int Input
// CHECK:  %gl_BaseInstance = OpVariable %_ptr_Input_int Input
// CHECK: %out_var_SV_InstanceID = OpVariable %_ptr_Output_int Output

// CHECK:                     %main = OpFunction
// CHECK:            %SV_InstanceID = OpVariable %_ptr_Function_int Function
// CHECK: [[gl_InstanceIndex:%[0-9]+]] = OpLoad %int %gl_InstanceIndex
// CHECK:  [[gl_BaseInstance:%[0-9]+]] = OpLoad %int %gl_BaseInstance
// CHECK:      [[instance_id:%[0-9]+]] = OpISub %int [[gl_InstanceIndex]] [[gl_BaseInstance]]
// CHECK:                             OpStore %SV_InstanceID [[instance_id]]
// CHECK:      [[instance_id_0:%[0-9]+]] = OpLoad %int %SV_InstanceID
// CHECK:                             OpStore %param_var_input [[instance_id_0]]
// CHECK:                  {{%[0-9]+}} = OpFunctionCall %int %src_main %param_var_input

int main(int input: SV_InstanceID) : SV_InstanceID {
    return input;
}

