// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct PSOut {
    [[vk::location(1), vk::index(5)]] float4 a: SV_Target0;
    [[vk::location(0), vk::index(1)]] float4 b: SV_Target1;
};

PSOut main() {
    PSOut o = (PSOut)0;
    return o;
}

// CHECK: :4:7: error: dual-source blending should use vk::location 0
// CHECK: :4:24: error: dual-source blending only accepts 0 or 1 as vk::index
