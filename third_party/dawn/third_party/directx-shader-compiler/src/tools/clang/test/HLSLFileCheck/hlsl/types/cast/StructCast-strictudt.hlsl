// RUN: %dxc -E main -T vs_6_0 -HV 2021 %s | FileCheck %s

// CHECK: SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader feeding rasterizer

struct Vertex
{
    float4 position     : POSITION0;
    float4 color        : COLOR0;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

struct Interpolants
{
    float4 position     : SV_POSITION0;
    float4 color        : COLOR0;

uint   RTIndex      : SV_RenderTargetArrayIndex;
};

struct Inh : Interpolants {
    float a;
};

struct Interpolants2
{
    float4 position     : SV_POSITION0;
    float4 color        : COLOR0;

    float4 color2        : COLOR2;
};


Interpolants2 c2;
Inh  c;
int i;
uint4 i4;

Interpolants main(  Vertex In)
{
  if (i>1)
     return (Interpolants)i4.z;
  else if (i>0)
     return (Interpolants) c;
  else if (i > -1)
     return (Interpolants )c2;
  return (Interpolants)In;
}
