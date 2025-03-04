// RUN: %dxc -T ps_6_0 -E PSMain -fcgl  %s -spirv | FileCheck %s

struct PSInput
{
        float4 color : COLOR;
};

// CHECK: OpFunctionCall %v4float %src_PSMain
float4 PSMain(PSInput input) : SV_TARGET
{
        return input.color;
}
