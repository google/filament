// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability ShaderViewportIndexLayerEXT
// CHECK:      OpExtension "SPV_EXT_shader_viewport_index_layer"

// CHECK:      OpEntryPoint Vertex %main "main"
// CHECK-SAME: %in_var_SV_RenderTargetArrayIndex
// CHECK-SAME: %gl_Layer

// CHECK:      OpDecorate %gl_Layer BuiltIn Layer
// CHECK:      OpDecorate %in_var_SV_RenderTargetArrayIndex Location 0

// CHECK:      %in_var_SV_RenderTargetArrayIndex = OpVariable %_ptr_Input_uint Input
// CHECK:      %gl_Layer = OpVariable %_ptr_Output_uint Output

uint main(uint input: SV_RenderTargetArrayIndex) : SV_RenderTargetArrayIndex {
    return input;
}
