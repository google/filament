// RUN: %dxilver 1.1 | %dxc -E main -T ds_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 3, outputs: 4, patchconst: 28
// CHECK: Outputs dependent on ViewId: { 0 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 1 }
// CHECK:   output 3 depends on inputs: { 1 }
// CHECK: PCInputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 7 }
// CHECK:   output 1 depends on inputs: { 26 }

struct DSFoo
{
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
    float4 a : AAA;
    float3 b[3] : BBB;
};

struct HSFoo
{
    float3 pos : AAA_HSFoo;
};

uint g_Idx1;

[domain("quad")]
float4 main(OutputPatch<HSFoo, 16> p, DSFoo input, float2 UV : SV_DomainLocation, uint vid : SV_ViewID) : SV_Position
{
    float4 r = 0;

    r += p[g_Idx1].pos.yyyy;
    r.x += input.Edges[1] + vid + UV.x;
    r.y = input.a.z;

    return r;
}
