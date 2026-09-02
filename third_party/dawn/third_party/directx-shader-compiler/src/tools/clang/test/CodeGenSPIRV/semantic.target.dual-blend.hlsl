// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %out_var_SV_Target0 Location 0
// CHECK: OpDecorate %out_var_SV_Target0 Index 0
// CHECK: OpDecorate %out_var_SV_Target1 Location 0
// CHECK: OpDecorate %out_var_SV_Target1 Index 1

struct PSOut {
    [[vk::location(0), vk::index(0)]] float4 a: SV_Target0;
    [[vk::location(0), vk::index(1)]] float4 b: SV_Target1;
};

PSOut main() {
    PSOut o = (PSOut)0;
    return o;
}
