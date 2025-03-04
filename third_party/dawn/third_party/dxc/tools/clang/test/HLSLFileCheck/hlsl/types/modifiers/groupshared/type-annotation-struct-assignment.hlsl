// RUN: %dxc -E cs_main -T cs_6_0 %s | FileCheck %s

// Regression test for crashing due to usage of structs as shared
// memory elements.

// CHECK: @cs_main

struct Struct
{
    uint4 A;
    float B;
};

groupshared Struct g_struct[2];

[RootSignature("")]
[numthreads(1, 1, 1)]
void cs_main()
{
    g_struct[0] = g_struct[1];
}
