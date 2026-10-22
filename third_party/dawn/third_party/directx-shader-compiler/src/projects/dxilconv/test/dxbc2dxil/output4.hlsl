// FXC command line: fxc /T vs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




void main(
            float4 a : A
           ,out float4 pos : SV_Position
           ,out min16float cull1 : SV_CullDistance0
           ,out float2 clip1 : SV_ClipDistance
           ,out min16float2 cull2 : SV_CullDistance1
           )
{
    float4 r = 0;
    r += a;
    pos = r;
    cull1 = r.x;
    cull2 = r.yz;
    clip1 = r.xw;
}
