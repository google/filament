// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: AtomicAdd
// CHECK: AtomicUMin
// CHECK: AtomicUMax
// CHECK: AtomicAnd
// CHECK: AtomicOr
// CHECK: AtomicXor
// CHECK: AtomicAdd
// CHECK: AtomicAdd
// CHECK: AtomicAdd
// CHECK: AtomicAdd
// CHECK: AtomicAdd
// CHECK: atomicCompareExchange
// CHECK: AtomicAdd
// CHECK: AtomicAdd
// CHECK: AtomicUMin
// CHECK: AtomicUMax
// CHECK: AtomicAnd
// CHECK: AtomicOr
// CHECK: AtomicXor
// CHECK: AtomicExchange
// CHECK: atomicCompareExchange

RWBuffer<uint> buf0;
RWByteAddressBuffer buf1;
RWByteAddressBuffer buf2[4][7];
RWTexture2D<uint> tex0;
RWTexture3D<uint> tex1;
RWTexture2DArray<uint> tex2;
RWTexture3D<uint> tex3[8][4];

#define RS "DescriptorTable(" \
             "UAV(u0), "\
             "UAV(u1), "\
             "UAV(u2, numDescriptors=28), "\
             "UAV(u30), "\
             "UAV(u31), "\
             "UAV(u32), "\
             "UAV(u33, numDescriptors=32) "\
             ")"\

[RootSignature( RS )]

float4 main(uint4 a : A, float4 b : B) : SV_Target
{
    uint4 r = a;
    uint comparevalue = r.w;
    uint newvalue = r.z;
    uint origvalue = buf0[r.z];

    InterlockedAdd(buf0[r.z], newvalue);
    InterlockedMin(buf0[r.z], newvalue);
    InterlockedMax(buf0[r.z], newvalue);
    InterlockedAnd(buf0[r.z], newvalue);
    InterlockedOr (buf0[r.z], newvalue);
    InterlockedXor(buf0[r.z], newvalue);

    InterlockedAdd(buf0[r.z], newvalue, origvalue); newvalue += origvalue;
    InterlockedAdd(tex0[r.xy], newvalue);
    InterlockedAdd(tex1[r.ywz], newvalue);
    InterlockedAdd(tex2[r.xyz], newvalue);
    InterlockedAdd(tex3[r.x][1][r.xyz], newvalue);

    InterlockedCompareExchange(buf0[r.z], comparevalue, newvalue, origvalue); newvalue += origvalue;

    buf1.InterlockedAdd(r.x, r.z, newvalue); // coord, newvalue, original
    buf2[2][r.y].InterlockedAdd(r.x, r.z, newvalue);
    buf1.InterlockedMin(r.x, r.z, newvalue);
    buf1.InterlockedMax(r.x, r.z, newvalue);
    buf1.InterlockedAnd(r.x, r.z, newvalue);
    buf1.InterlockedOr (r.x, r.z, newvalue);
    buf1.InterlockedXor(r.x, r.z, newvalue);
    buf1.InterlockedExchange(r.z, newvalue, origvalue); newvalue += origvalue;
    buf1.InterlockedCompareExchange(r.z, comparevalue, newvalue, origvalue); newvalue += origvalue;

    return newvalue;
}
