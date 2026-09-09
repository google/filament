// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: frem
// CHECK: frem
// CHECK: frem
// CHECK: frem

float4 g_constants;


AppendStructuredBuffer<float4> outputUAV : register(u0);

float4 main() : SV_TARGET
{
    float4 x = g_constants;
    float y = g_constants[1];

    outputUAV.Append(asuint(x % y));
    return 1.0;
}
