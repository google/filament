// RUN: %dxc -T vs_6_4 -E main -fcgl  %s -spirv | FileCheck %s

void main(out uint rate : SV_ShadingRate) {
// CHECK:   OpCapability FragmentShadingRateKHR
// CHECK:   OpExtension "SPV_KHR_fragment_shading_rate"
// CHECK:   OpDecorate [[r:%[0-9]+]] BuiltIn PrimitiveShadingRateKHR
// CHECK:   [[r_0:%[0-9]+]] = OpVariable %_ptr_Output_uint Output
    rate = 0;
}
