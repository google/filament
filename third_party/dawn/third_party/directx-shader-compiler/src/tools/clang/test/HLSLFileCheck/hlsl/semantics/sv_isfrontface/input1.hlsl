// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// For IsFrontFace.
// CHECK: icmp ne

float4 main(
            float4 a : A
           ,int4 b : B
           ,uint4 c : C
           ,min16float4 d : D
           ,float cl : SV_ClipDistance
           ,float cu : SV_CullDistance
           ,uint icov : SV_Coverage
           ,uint instid : SV_InstanceID
           ,uint primid : SV_PrimitiveID
           ,uint rtai : SV_RenderTargetArrayIndex
           ,uint vpai : SV_ViewportArrayIndex
           ,float4 pos : SV_Position
           // SGV
           ,uint sampidx : SV_SampleIndex
           ,bool isFF: SV_IsFrontFace
           ) : SV_Target
{
    float4 r = 0;
    r += a;
    r += b;
    r += c;
    r += d;
    r += cl;
    r += cu;
    r += icov;
    r += instid;
    r += primid;
    r += rtai;
    r += vpai;
    r += pos;

    r += sampidx;
    r += isFF;
    return r;
}
