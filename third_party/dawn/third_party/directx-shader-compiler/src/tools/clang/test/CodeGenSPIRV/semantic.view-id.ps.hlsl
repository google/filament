// RUN: %dxc -T ps_6_1 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability MultiView
// CHECK:      OpExtension "SPV_KHR_multiview"

// CHECK:      OpEntryPoint Fragment
// CHECK-SAME: [[viewindex:%[0-9]+]]

// CHECK:      OpDecorate [[viewindex]] BuiltIn ViewIndex

// CHECK:      [[viewindex]] = OpVariable %_ptr_Input_uint Input

float4 main(uint viewid: SV_ViewID) : SV_Target {
    return viewid;
}
