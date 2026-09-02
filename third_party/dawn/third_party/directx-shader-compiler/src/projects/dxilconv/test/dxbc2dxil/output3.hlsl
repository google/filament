// FXC command line: fxc /T ps_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




void main(
            float4 a : A
           // SGV
           ,out float depthle : SV_DepthLessEqual
           ,out min16uint stencil : SV_StencilRef
           ,out float4 rt0 : SV_Target0
           ,out float4 rt5 : SV_Target5
           )
{
    float4 r = 0;
    r += a;
    depthle = r.x;
    stencil = r.x;
    rt0 = r;
    rt5 = r;
}
