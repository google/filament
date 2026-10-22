// RUN: %dxc -E main -T vs_6_0 -spirv -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability ShaderLayer
// CHECK-NOT:  OpExtension "SPV_EXT_shader_viewport_index_layer"

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
