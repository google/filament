// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability Geometry

// CHECK:      OpEntryPoint Fragment %main "main"
// CHECK-SAME: %gl_PrimitiveID

// CHECK:      OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId

// CHECK:      %gl_PrimitiveID = OpVariable %_ptr_Input_uint Input

float4 main(uint id : SV_PrimitiveID) : SV_Target {
    return id;
}
