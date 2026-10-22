// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability MultiViewport

// CHECK:      OpEntryPoint Fragment %main "main"
// CHECK-SAME: %gl_ViewportIndex

// CHECK:      OpDecorate %gl_ViewportIndex BuiltIn ViewportIndex

// CHECK:      %gl_ViewportIndex = OpVariable %_ptr_Input_uint Input

float4 main(uint input: SV_ViewportArrayIndex) : SV_Target {
    return input;
}
