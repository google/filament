// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability ShaderViewportIndexLayerEXT
// CHECK:      OpExtension "SPV_EXT_shader_viewport_index_layer"

// CHECK:      OpEntryPoint Vertex %main "main"
// CHECK-SAME: %in_var_SV_ViewportArrayIndex
// CHECK-SAME: %gl_ViewportIndex

// CHECK:      OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex
// CHECK:      OpDecorate %in_var_SV_ViewportArrayIndex Location 0

// CHECK:      %in_var_SV_ViewportArrayIndex = OpVariable %_ptr_Input_uint Input
// CHECK:      %gl_ViewportIndex = OpVariable %_ptr_Output_uint Output

uint main(uint input: SV_ViewportArrayIndex) : SV_ViewportArrayIndex {
    return input;
}
