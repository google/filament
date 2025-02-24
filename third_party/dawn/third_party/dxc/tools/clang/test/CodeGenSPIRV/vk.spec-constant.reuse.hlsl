// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %y SpecId 0
[[vk::constant_id(0)]] const bool y = false;

[shader("pixel")]
float4 main(float4 position : SV_Position) : SV_Target0 {
// CHECK: OpConstantComposite %v4bool %y %y %y %y
    return y ? position : 1.0;
}
