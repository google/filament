// RUN: %dxc -T ps_6_0 -E main -WX %s | FileCheck %s

// CHECK: semantic 'COLOR0' on field overridden by function or enclosing type

struct Output
{
    float a;
    float b : COLOR0;
};

Output main() : SV_Target
{
    Output output;
    output.a = 1;
    output.b = 2;
    return output;
}