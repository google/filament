// RUN: %dxc -T vs_6_0 -E main -fvk-support-nonzero-base-vertex -fcgl  %s -spirv | FileCheck %s

// CHECK:                     OpEntryPoint Vertex %main "main"
// CHECK-SAME:                %gl_VertexIndex
// CHECK-SAME:                [[gl_BaseVertex:%[0-9]+]]
// CHECK-SAME:                %out_var_A

// CHECK:                     OpDecorate %gl_VertexIndex BuiltIn VertexIndex
// CHECK:                     OpDecorate [[gl_BaseVertex]] BuiltIn BaseVertex
// CHECK:                     OpDecorate %out_var_A Location 0

// CHECK: %gl_VertexIndex = OpVariable %_ptr_Input_uint Input
// CHECK: [[gl_BaseVertex]] = OpVariable %_ptr_Input_uint Input
// CHECK: %out_var_A = OpVariable %_ptr_Output_uint Output

// CHECK:                     %main = OpFunction
// CHECK:            %SV_VertexID = OpVariable %_ptr_Function_uint Function
// CHECK: [[gl_VertexIndex:%[0-9]+]] = OpLoad %uint %gl_VertexIndex
// CHECK:  [[base_vertex:%[0-9]+]] = OpLoad %uint [[gl_BaseVertex]]
// CHECK:      [[vertex_id:%[0-9]+]] = OpISub %uint [[gl_VertexIndex]] [[base_vertex]]
// CHECK:                             OpStore %SV_VertexID [[vertex_id]]
// CHECK:      [[vertex_id_0:%[0-9]+]] = OpLoad %uint %SV_VertexID
// CHECK:                             OpStore %param_var_input [[vertex_id_0]]
// CHECK:                  {{%[0-9]+}} = OpFunctionCall %uint %src_main %param_var_input

unsigned int main(unsigned int input: SV_VertexID) : A {
    return input;
}

