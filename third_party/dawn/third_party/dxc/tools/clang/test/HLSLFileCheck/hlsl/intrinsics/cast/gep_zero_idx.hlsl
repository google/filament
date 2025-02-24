// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: bitcast

uint uintConstant;
uint i;
float4 main() : SV_Target
{
    float a[2];

    a[0] = asfloat(uintConstant);
    a[1] = 0;

    return a[i];
}