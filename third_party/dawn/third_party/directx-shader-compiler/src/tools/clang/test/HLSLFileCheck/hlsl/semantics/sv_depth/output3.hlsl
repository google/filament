// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types
// CHECK: PS Output Stencil Ref

// Make sure depth is after target.
// CHECK: Output signature
// CHECK: SV_Target
// CHECK: SV_DepthLessEqual

// CHECK: DepthOutput=1

// CHECK: uitofp i16

void main(
            float4 a : A,
            min16uint b : B
           // SGV
           ,out float depthle : SV_DepthLessEqual
           ,out uint stencil : SV_StencilRef
           ,out float4 rt0 : SV_Target0
           ,out float4 rt5 : SV_Target5
           )
{
    float4 r = 0;
    r += a + b;
    depthle = r.x;
    stencil = r.x;
    rt0 = r;
    rt5 = r;
}
