// FXC command line: fxc /T hs_5_1 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




// fxc /dev od 1 /T hs_5_1 hs1.hlsl

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

//[RootSignature("SRV(t3)")]
DSFoo PatchFoo(InputPatch<HSFoo, 16> ip, uint PatchID : SV_PrimitiveID)
{
    DSFoo a;
    a.Edges[0] = ip[PatchID].pos.x;
    a.Edges[1] = ip[PatchID].pos.y;
    a.Edges[2] = ip[PatchID].pos.z;
    a.Edges[3] = 0;
    a.Inside[0] = ip[PatchID].pos.z;
    a.Inside[1] = ip[PatchID].pos.z;
    //a.Inside[1] += mycb0.b;
    //a.Inside[1] += mycb1[mycb0.a.y].a.z;
    a.Inside[1] += mycb1[2].a.z;
    return a;
}

[RootSignature("DescriptorTable(CBV(b0), CBV(b0, space=1, numDescriptors=8), SRV(t20, space=10, numDescriptors=10), UAV(u5, space=7, numDescriptors=10)), DescriptorTable(Sampler(s10, space=3, numDescriptors=10))")]
[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PatchFoo")]
HSFoo main( InputPatch<HSFoo, 16> p, 
            uint i : SV_OutputControlPointID,
            uint PatchID : SV_PrimitiveID )
{
    HSFoo output;
    float4 r = float4(p[PatchID].pos, 1);
    r += tex1[r.x].Load(r.xyz);
    //r += mycb1[1].a;
    //r += mycb1[mycb0.b].a;
    r += mycb0.b;
    output.pos = p[i].pos + r.xyz;
    return output;
}


