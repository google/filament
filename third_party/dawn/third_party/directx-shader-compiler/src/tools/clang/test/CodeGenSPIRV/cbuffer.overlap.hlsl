// RUN: not %dxc -T vs_6_2 -E main -fcgl  %s -spirv  2>&1 | FileCheck %s

// CHECK: error: field "gFoo" at register(c5) overlaps with previous members

uniform float4x4 gMVP : register(c0);
uniform float4   gFoo : register(c5);
uniform float4   gBar : register(c5);

float4 main(float4 pos : POSITION) : SV_Position {
    return mul(gMVP, pos * gFoo + gBar);
}
