// RUN: %dxc -E main -T ps_6_0 -Gis %s | FileCheck %s

// CHECK-NOT: fadd fast

float4 main(float4 a : A) : SV_Target
{
    float t = 2.f;
    float4 t2 = a + t + (float)2.f;
    return t2+3;
}
