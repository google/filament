// RUN: dxc /Tvs_6_0 /Od %s | FileCheck %s

// This is a regression test for a crash in dxilgen when compiling
// with optimization disabled. It triggered a crash because the
// gep used by the subscript operator was fed through an lcssa
// phi for a use out of the loop and phis are not allowed users
// of the gep for the subscript operator.

// CHECK: call %dx.types.ResRet.i32 @dx.op.bufferLoad.i32

struct S { uint bits; };
StructuredBuffer<S> buf : register(t3);

S get(uint idx)
{
    return buf[idx];
}

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=10))")]
uint main(uint C : A) : Z
{
    int i = 0;
    S s;
    do {
        s = get(i);
    } while (i < C);
    
    return s.bits;
}