// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: if statement conditional expressions must evaluate to a scalar

float4 m;
float4 main() : SV_Target
{
    if (m)
        return 0;
    else
        return 1;
}