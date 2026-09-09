// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability Geometry

// CHECK:      OpEntryPoint Fragment %main "main"
// CHECK-SAME: %gl_Layer

// CHECK:      OpDecorate %gl_Layer BuiltIn Layer

// CHECK:      %gl_Layer = OpVariable %_ptr_Input_uint Input

float4 main(uint input: SV_RenderTargetArrayIndex) : SV_Target {
    return input;
}
