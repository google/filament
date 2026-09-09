// RUN: %dxc -T ps_6_0 -E main -WX %s | FileCheck %s

// CHECK: semantic 'NORMAL0' on field overridden by function or enclosing type

struct Input
{
    float a;
    float b : NORMAL0;
};

float main(Input input : TEXCOORD0) : SV_Target
{
    return 1;
}