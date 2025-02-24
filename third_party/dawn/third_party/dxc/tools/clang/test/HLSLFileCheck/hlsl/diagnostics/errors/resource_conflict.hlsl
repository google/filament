// RUN: %dxc -E main -T ps_6_0 %s -Zi | FileCheck %s

// CHECK: error: resource b at register 5 overlaps with resource a at register 5
// CHECK-NOT: Internal Compiler error

Texture2D a : register(t5);
Texture2D b : register(t5);
SamplerState s : register(s0);

float4 main(float2 uv : TEXCOORD) : SV_Target {
    return a.Sample(s, uv) + b.Sample(s, uv);
}
