// RUN: %dxc -E main -T vs_6_0 -HV 2018 %s | FileCheck %s

// CHECK: switch

struct Vertex
{
    float4 position     : POSITION0;
    float4 color        : COLOR0;
    float4 a[3]         : A;
};

struct Interpolants
{
    float4 position     : SV_POSITION0;
    float4 color        : COLOR0;
    float4 a[3]         : A;
};

uint i;

void main( Vertex In, int j : J, out Interpolants output )
{
    output = In;
    output.a[1][i] = 3;
    output.a[0] = 4;
}
