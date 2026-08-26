// RUN: %dxc -T vs_6_1 -E main -fspv-extension=KHR -fcgl  %s -spirv | FileCheck %s

// CHECK: OpExtension "SPV_KHR_shader_draw_parameters"
// CHECK: OpExtension "SPV_KHR_device_group"
// CHECK: OpExtension "SPV_KHR_multiview"

float4 main(
    [[vk::builtin("BaseVertex")]]    int baseVertex  : A,
   [[vk::builtin("DeviceIndex")]]    int deviceIndex : C,
                                     uint viewid     : SV_ViewID) : B {
    return baseVertex + viewid;
}
