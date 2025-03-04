// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Typed UAV Load Additional Formats

struct PSInput {
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

RWTexture2D<float2> g_tex : register(u0);

float4 main(PSInput input) : SV_TARGET {
    float2 val = g_tex.Load(input.uv);
    return val.xyxx;
}