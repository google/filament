// RUN: %dxc -T vs_6_1 -E main -fspv-extension=SPV_KHR_multiview -fspv-extension=SPV_KHR_shader_draw_parameters -fcgl  %s -spirv | FileCheck %s

// CHECK:      OpExtension "SPV_KHR_shader_draw_parameters"
// CHECK:      OpExtension "SPV_KHR_multiview"

float4 main(
    [[vk::builtin("BaseVertex")]]    int baseVertex : A,
                                     uint viewid    : SV_ViewID) : B {
    return baseVertex + viewid;
}
