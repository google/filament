// RUN: %dxc -T ps_6_0 -E PSMain -Zi -fcgl  %s -spirv | FileCheck %s

float4 PSMain(float4 color : COLOR) : SV_TARGET { return color; }
// CHECK: float4 PSMain(float4 color : COLOR) : SV_TARGET { return color; }
