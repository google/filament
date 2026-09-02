// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
float4 a;

static float4 m = {a};

float4 main() : SV_Target {
    return m;
}