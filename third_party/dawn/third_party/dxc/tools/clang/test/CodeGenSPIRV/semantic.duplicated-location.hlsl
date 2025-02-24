// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

struct PSInput
{
    float4 color : COLOR;
};

// CHECK: error: Multiple stage variables have a duplicated pair of location and index at 0 / 0

void main(PSInput input,
        out float4 x : SV_Target,
        out float4 y : SV_Target0,
        out float4 z : SV_Target1)
{
    x = input.color;
    y = input.color;
    z = input.color;
}

