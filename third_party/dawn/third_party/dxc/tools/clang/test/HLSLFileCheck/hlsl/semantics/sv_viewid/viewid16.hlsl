// RUN: %dxilver 1.1 | %dxc -E main -T hs_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 3, outputs: 3, patchconst: 24
// CHECK: Outputs dependent on ViewId: { 0, 2 }
// CHECK: PCOutputs dependent on ViewId: { 15 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0 }
// CHECK:   output 1 depends on inputs: { 0, 1 }
// CHECK:   output 2 depends on inputs: { 0, 2 }
// CHECK: Inputs contributing to computation of PCOutputs:
// CHECK:   output 3 depends on inputs: { 0 }
// CHECK:   output 7 depends on inputs: { 1 }
// CHECK:   output 11 depends on inputs: { 0, 2 }
// CHECK:   output 19 depends on inputs: { 2 }
// CHECK:   output 23 depends on inputs: { 2 }

struct Foo1
{
    int b;
    float4x4 m1[3];
    float c;
    float4 a;
    float4x4 m2[3];
};

Texture2D<float4> tex1[10] : register( t20, space10 );
SamplerState samp1[8] : register(s10, space3);
RWBuffer<float4> buf1[10] : register( u5, space7 );

ConstantBuffer<Foo1> mycb0      : register(b0, space0);
ConstantBuffer<Foo1> mycb1[8]   : register(b0, space1);

struct DSFoo
{
    float Edges[4]  : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

struct HSFoo
{
    float3 pos : POSITION;
};

DSFoo PatchFoo(InputPatch<HSFoo, 16> ip, uint PatchID : SV_PrimitiveID, uint vid : SV_ViewID)
{
    DSFoo a;
    a.Edges[0] = ip[PatchID].pos.x;
    a.Edges[1] = ip[PatchID].pos.y;
    a.Edges[2] = ip[PatchID].pos.z + ip[PatchID].pos.x;
    a.Edges[3] = vid;
    a.Inside[0] = ip[PatchID].pos.z;
    a.Inside[1] = ip[PatchID].pos.z;
    a.Inside[1] += mycb1[2].a.z;
    return a;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PatchFoo")]
HSFoo main(InputPatch<HSFoo, 16> p,
           uint i : SV_OutputControlPointID,
           uint vid : SV_ViewID,
           uint PatchID : SV_PrimitiveID)
{
    HSFoo output;
    float4 r = float4(p[PatchID].pos.xxx, 1);
    r += tex1[r.x].Load(r.xyz);
    r.x += p[vid].pos.x;
    if (p[PatchID].pos.z == 1.f || vid == 3)
      r.z += mycb0.b;
    output.pos = p[i].pos.xyx + r.xyz;
    return output;
}
