// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: DepthOutput=1
// CHECK: SampleFrequency=1

void main(
            float4 a : A
           // SGV
           ,uint sampidx : SV_SampleIndex
           ,bool isFF: SV_IsFrontFace
           ,out float depth : SV_Depth
           ,out float4 rt0 : SV_Target0
           ,out float4 rt5 : SV_Target5
           )
{
    float4 r = 0;
    r += a;
    r += sampidx;
    r += isFF;
    depth = r.x;
    rt0 = r;
    rt5 = r;
}
