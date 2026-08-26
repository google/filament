// RUN: %dxc -E main -T hs_6_0 %s | FileCheck %s
// RUN: %dxc -E main -T hs_6_0 %s | %D3DReflect %s | FileCheck -check-prefix=REFL %s

// CHECK: float 3.000000e+00}

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

DSFoo PatchFoo(InputPatch<HSFoo_Input, 32> ip, OutputPatch<HSFoo, 16> op, uint PatchID : SV_PrimitiveID)
{
    DSFoo a;
    a.Edges[0] = ip[PatchID].qq.y;
    a.Edges[1] = ip[PatchID].qq.y;
    a.Edges[2] = ip[PatchID].qq.y;
    a.Edges[3] = ip[PatchID].qq.y;
#if 0
    a.Edges[0] = ip[PatchID].qq.y + 2;
    a.Edges[1] = ip[PatchID].qq.y - 3;
    a.Edges[2] = ip[PatchID].qq.y + 4;
    a.Edges[3] = ip[PatchID].qq.y - 5;
#endif

    a.Inside[0] = ip[PatchID].qq.w;
    a.Inside[1] = ip[PatchID].qq.w;
    //a.Inside[1] += mycb0.b;
    //a.Inside[1] += mycb1[mycb0.a.y].a.z;
    //a.Inside[1] += mycb1[2].a.z;

    a.a = 0;
    for (int i = 0; i < 32; i++)
    {
      a.a += ip[i].qq.x * ip[i].qq.y /* * ip[i].qq.z */ + a.Edges[i%4];
      a.a += op[i/2].d.x;
      //op[i].d.y = a.a;
    }

    a.Inside[0] += a.a;
    a.Inside[1] += a.a;

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
            uint PatchID : SV_PrimitiveID )
{
    HSFoo output;
    float4 r = p[PatchID].qq;
    output.d = p[i].qq + r;
    return output;
}

// REFL: TempArrayCount: 16
// REFL: DynamicFlowControlCount: 1
// REFL: ArrayInstructionCount: 5
// REFL: TextureNormalInstructions: 0
// REFL: TextureLoadInstructions: 0
// REFL: TextureCompInstructions: 0
// REFL: TextureBiasInstructions: 0
// REFL: TextureGradientInstructions: 0
// REFL: UintInstructionCount: 0
// REFL: CutInstructionCount: 0
// REFL: EmitInstructionCount: 0
// REFL: cBarrierInstructions: 0
// REFL: cInterlockedInstructions: 0
// REFL: cTextureStoreInstructions: 0
