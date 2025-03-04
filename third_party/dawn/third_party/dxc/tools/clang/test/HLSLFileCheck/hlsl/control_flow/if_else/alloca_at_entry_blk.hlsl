// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
int i;
float4 v;
float4 main() : SV_Target
{
    float4 vec = 0;

    if (i > 0)
        vec = v.xyzw[i];

    return vec;
}