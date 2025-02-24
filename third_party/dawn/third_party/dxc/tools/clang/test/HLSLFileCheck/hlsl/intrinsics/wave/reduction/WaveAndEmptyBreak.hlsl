// RUN: %dxc -T cs_6_0 %s | FileCheck %s

// Test a case that might appear to require dx.break, but ultimately does not.
// CHECK-NOT: dx.break

RWStructuredBuffer<uint> u0;
Texture1D<uint> t0;
SamplerComparisonState  s0;
[RootSignature("DescriptorTable(SRV(t0), UAV(u0)), DescriptorTable(Sampler(s0))")]
[numthreads(64, 1, 1)]
void main(uint GI : SV_GroupIndex)
{
    uint RetMask;
    uint GIKey = t0[GI];
    uint Mask = t0[GI+1];
    for ( ;; )
    {
        const uint FirstKey = WaveReadLaneFirst(GIKey);

        RetMask = WaveReadLaneAt(Mask, FirstKey);
        if (FirstKey == GIKey) break;
    }
    u0[GI] = RetMask.x;
}
