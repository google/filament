// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: icmp ne i32 {{.*}}, 0
// CHECK: fcmp fast une float {{.*}}, 0.000000e+00
// CHECK: fcmp fast une float {{.*}}, 0.000000e+00
// CHECK: fcmp fast une float {{.*}}, 0.000000e+00
// CHECK: fcmp fast une float {{.*}}, 0.000000e+00

struct Input
{
    int2x2 i : I;
    float2x2 f : F;
};

struct Output
{
    bool2x2 i : I;
    bool2x2 f : F;
};

Output main(Input input)
{
    Output output;
    output.i = (bool2x2)input.i;
    output.f = (bool2x2)input.f;
    return output;
}