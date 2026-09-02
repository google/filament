// RUN: %dxilver 1.1 | %dxc -E main -T hs_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 4, outputs: 4, patchconst: 28
// CHECK: Outputs dependent on ViewId: { 2 }
// CHECK: PCOutputs dependent on ViewId: { 7, 23 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0 }
// CHECK:   output 1 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 0, 2, 3 }
// CHECK:   output 3 depends on inputs: { 3 }
// CHECK: Inputs contributing to computation of PCOutputs:
// CHECK:   output 3 depends on inputs: { 1 }
// CHECK:   output 7 depends on inputs: { 1 }
// CHECK:   output 11 depends on inputs: { 1 }
// CHECK:   output 15 depends on inputs: { 1 }
// CHECK:   output 19 depends on inputs: { 0, 3 }
// CHECK:   output 23 depends on inputs: { 0, 2, 3 }

struct DSFoo
{
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
    float4 a : AAA;
};

struct HSFoo
{
    float4 d : Sem_HSFoo;
};

struct HSFoo_Input
{
    float4 qq : Sem_HSFoo_Input_qq;
};

DSFoo PatchFoo(InputPatch<HSFoo_Input, 32> ip, OutputPatch<HSFoo, 16> op, uint PatchID : SV_PrimitiveID, uint vid : SV_ViewID)
{
    DSFoo a;
    a.Edges[0] = ip[PatchID].qq.y;
    a.Edges[1] = ip[vid].qq.y;
    a.Edges[2] = ip[PatchID].qq.y;
    a.Edges[3] = ip[PatchID].qq.y;

    a.Inside[0] = ip[PatchID].qq.w + op[PatchID].d.x;
    a.Inside[1] = op[PatchID].d.z;

    return a;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[maxtessfactor(3.f)]
[patchconstantfunc("PatchFoo")]
HSFoo main( InputPatch<HSFoo_Input, 32> p, 
            uint i : SV_OutputControlPointID,
            uint PatchID : SV_PrimitiveID,
            uint vid : SV_ViewID )
{
    HSFoo output;
    float4 r = p[PatchID].qq;
    r.z += p[PatchID].qq.x + p[PatchID].qq.w + vid;
    output.d = p[i].qq + r;
    return output;
}
