// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// CHECK: atomicrmw or
// CHECK: atomicrmw or
// CHECK: 3)
// CHECK: atomicrmw or
// CHECK: 2)
// CHECK: atomicrmw or
// CHECK: 3)
// CHECK: atomicBinOp
// CHECK: i32 12,
// CHECK: atomicBinOp
// CHECK: i32 4,
// CHECK: atomicBinOp
// CHECK: i32 12,


groupshared row_major uint2x2 dataC[8*8];
groupshared uint4 a;

RWStructuredBuffer<uint2x2> mats;
RWBuffer<uint4> bufA;

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    dataC[tid.x%(8*8)] = mats.Load(gid.x);
    GroupMemoryBarrier();
    if (tid.x == 7) {
       a = 3;
    }
    GroupMemoryBarrierWithGroupSync();
    InterlockedOr(a.y, 1);
    InterlockedOr(dataC[0][1].y, 2);
    InterlockedOr(dataC[0][1][0], 3);
    InterlockedOr(dataC[0]._m11, 4);
    bufA[tid.x] = a;
    mats[tid.x] = dataC[tid.x%(8*8)];

    InterlockedOr(mats[0][1].y, 2);
    InterlockedOr(mats[0][1][0], 3);
    InterlockedOr(mats[0]._m11, 4);
}
