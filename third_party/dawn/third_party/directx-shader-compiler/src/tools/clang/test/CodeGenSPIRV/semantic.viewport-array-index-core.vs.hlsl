// RUN: %dxc -E main -T vs_6_0 -spirv -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability ShaderViewportIndex
// CHECK-NOT:  OpExtension "SPV_EXT_shader_viewport_index_layer"

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
