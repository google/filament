// RUN: %dxc -T ps_6_0 -E main -O0  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_arr__struct_12_uint_2 ArrayStride 16

struct PSInput
{
    float4 color : COLOR;
};

cbuffer foo {
    struct {
    } bar[2];
}

float4 main(PSInput input) : SV_TARGET
{
    return input.color;
}
