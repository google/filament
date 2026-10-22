// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main
struct T {
float m;
};
T t;
static float s = 2;

float main(float2 a : A, float b : B) : SV_Target
{
    return t.m.x + s.xxxx;
}