// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
struct PSInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

SamplerState samp1;

Texture2D<float4> tex1;

float4 main(PSInput input) : SV_TARGET
{
    min16float2 v = min16float2(input.color.xy);
    uint2 u = f32tof16(v);
    float f = tex1.CalculateLevelOfDetail(samp1, v);
    return input.color + u.xyxy + f;
}