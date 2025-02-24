// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv 2>&1 | FileCheck %s

struct PSInput
{
    [[vk::location(0)]] double4 data1 : COLOR0; // double4 consumes 2 Locations (0 and 1)
    [[vk::location(1)]] double4 data2 : COLOR1;
    // CHECK: error: stage input location #1 already consumed by semantic 'COLOR0'
};

void main(PSInput input) : SV_TARGET
{
}
