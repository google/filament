// FXC command line: fxc /T vs_5_0 %s /Fo %t.dxbc
// RUN: %dxbc2dxil %t.dxbc /emit-llvm /o %t.ll.converted
// RUN: fc %b.ref %t.ll.converted




float4 main(
            float4 a : A
            // SGV
           ,uint vertid : SV_VertexID
           ,uint instid : SV_InstanceID
           ) : SV_Position
{
    float4 r = 0;
    r += a;
    r += vertid;
    r += instid;
    return r;
}
