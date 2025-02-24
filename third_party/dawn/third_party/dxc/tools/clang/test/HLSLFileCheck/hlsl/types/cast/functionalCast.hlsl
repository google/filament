// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

struct P {
   float4 x;
};

struct C : P {
    int4 y;
};

P p;

float4 x;
int4 y;
float4 main(float2 a : A, float b : B) : SV_Target
{
    C c;
    (P)c = p;
    c.y = y;
    return half4(x).x + c.x + c.y;
}
