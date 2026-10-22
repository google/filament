// FXC command line: fxc /T ds_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




// fxc /dev od 1 /T ds_5_0 ds1.hlsl

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
float4 main(OutputPatch<HSFoo, 16> p, DSFoo input, float2 UV : SV_DomainLocation) : SV_Position
{
    int i;
    float4 r = 0;
    
    [loop]
    for (i = 0; i < 16; i++)
    {
        r += p[i].pos.xyzz;
    }

    for (i = 0; i < 4; i++)
    {
        r += input.Edges[i]*(i+1);
    }
    r += input.Inside[0] + input.Inside[1];
    r += input.a.wyxz + input.b[g_Idx1].xyzy;

    r *= UV.xyyx;

    return r;
}

