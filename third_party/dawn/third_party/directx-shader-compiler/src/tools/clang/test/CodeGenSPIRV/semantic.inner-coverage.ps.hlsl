// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpCapability FragmentFullyCoveredEXT
// CHECK:      OpExtension "SPV_EXT_fragment_fully_covered"

// CHECK:      OpEntryPoint Fragment
// CHECK-SAME: [[coverage:%[0-9]+]]
// CHECK:      [[coverage]] = OpVariable %_ptr_Input_bool Input


float4 main(uint inCov : SV_InnerCoverage) : SV_Target {
// CHECK:      [[boolv:%[0-9]+]] = OpLoad %bool [[coverage]]
// CHECK-NEXT:  [[intv:%[0-9]+]] = OpSelect %uint [[boolv]] %uint_1 %uint_0
// CHECK-NEXT:                  OpStore %param_var_inCov [[intv]]
    return inCov;
}
