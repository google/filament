// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: PS Inner Coverage
// CHECKNOT: SV_RenderTargetArrayIndex or SV_ViewportArrayIndex from any shader feeding rasterizer

// CHECK: DepthOutput=1


void main(
            float4 a : A
           // SGV
           ,out float depthle : SV_DepthLessEqual
           ,uint cover : SV_InnerCoverage
           , uint index : SV_RenderTargetArrayIndex
           ,out float4 rt0 : SV_Target0
           ,out float4 rt5 : SV_Target5
           )
{
    float4 r = 0;
    r += a;
    depthle = r.x;
    rt0 = cover + index;
    rt5 = r;
}
