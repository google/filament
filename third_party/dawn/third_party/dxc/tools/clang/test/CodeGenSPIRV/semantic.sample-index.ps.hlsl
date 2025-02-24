// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability SampleRateShading

// CHECK:      OpEntryPoint Fragment %main "main"
// CHECK-SAME: %gl_SampleID

// CEHCK:      OpDecorate %gl_SampleID BuiltIn SampleId

// CHECK:      %gl_SampleID = OpVariable %_ptr_Input_uint Input

float4 main(uint index : SV_SampleIndex) : SV_Target {
    return 1.0;
}
