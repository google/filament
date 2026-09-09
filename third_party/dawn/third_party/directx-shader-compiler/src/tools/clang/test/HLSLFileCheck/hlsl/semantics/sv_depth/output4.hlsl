// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// CHECK: Minimum-precision data types

// CHECK: fptrunc float
// CHECK: to half
// CHECK: fptrunc float
// CHECK: to half
// CHECK: fptrunc float
// CHECK: to half

void main(
            float4 a : A
           ,out float4 pos : SV_Position           
           ,out min16float tcull1 : CullDistance0
           ,out float2 tclip1 : ClipDistance
           ,out min16float2 tcull2 : CullDistance1

           ,out float cull1 : SV_CullDistance0
           ,out float2 clip1 : SV_ClipDistance
           ,out float2 cull2 : SV_CullDistance1
           )
{
    float4 r = 0;
    r += a;
    pos = r;
    tcull1 = r.x;
    tcull2 = r.yz;
    tclip1 = r.xw;

    cull1 = r.x;
    cull2 = r.yz;
    clip1 = r.xw;
}
