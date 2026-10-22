// RUN: %dxc -T ps_6_4 -E main -fcgl  %s -spirv | FileCheck %s

float4 main(uint rate : SV_ShadingRate) : SV_TARGET {
// CHECK:   OpCapability FragmentShadingRateKHR
// CHECK:   OpExtension "SPV_KHR_fragment_shading_rate"
// CHECK:   OpDecorate [[r:%[0-9]+]] BuiltIn ShadingRateKHR
// CHECK:   [[r_0:%[0-9]+]] = OpVariable %_ptr_Input_uint Input
  return float4(rate, 0, 0, 0);
}
