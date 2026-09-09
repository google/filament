// FXC command line: fxc /Ths_5_0 %s /Fo %t.dxbc
// FXC command line: fxc /Ths_5_1 /DDX12 %s /Fo %t.DX12.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm | %FileCheck %s
// RUN: %dxbc2dxil %t.DX12.dxbc /emit-llvm | %FileCheck %s

// CHECK: hullloop0:
// CHECK: %[[InstanceID:.*]] = phi i32
// CHECK: sub i32 0, %[[InstanceID:.*]]

#ifndef DX12
#define RS
#else
#define RS [RootSignature("CBV(b2)")]
#endif

struct Vertex
{
    float4 pos : SV_POSITION;
    float4 c : COLOR;
};

struct HullShaderConstantData
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};


cbuffer Buf0 : register(b2) {
uint foo;
}

HullShaderConstantData ConstantFunction( InputPatch<Vertex, 5> In )
{
    HullShaderConstantData Out = (HullShaderConstantData)0;
    for (int i = 0; i<4; ++i) {
      Out.Edges[i] += In[foo - i].c.w;
    }    
    return Out;
}

RS
[domain("quad")]
[partitioning("integer")]
[outputtopology("point")]
[outputcontrolpoints(0)]
[patchconstantfunc("ConstantFunction")]
void main( InputPatch<Vertex, 5> In, uint i : SV_OutputControlPointID )
{
}
