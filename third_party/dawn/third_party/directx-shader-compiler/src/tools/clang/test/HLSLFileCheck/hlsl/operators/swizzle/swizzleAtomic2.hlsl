// RUN: %dxc -E main -T cs_6_0 %s | FileCheck %s

// Check that attempting to use atomic ops on swizzled typed resource members will fail
// Use representatives of atomic binops and cmpexchange type atomic operations.
// Include both typed buffer and texture resources

RWBuffer<uint4> bufA;
RWTexture1D<uint4> texA;

 // CHECK: error: Invalid operation on typed buffer.
// CHECK: error: Invalid operation on typed buffer.

[numthreads(8,8,1)]
void main( uint2 tid : SV_DispatchThreadID, uint2 gid : SV_GroupID, uint2 gtid : SV_GroupThreadID, uint gidx : SV_GroupIndex )
{
    // CHECK: Typed resources used in atomic operations must have a scalar element type
    // CHECK: Typed resources used in atomic operations must have a scalar element type
    bufA[tid.x] = gid.x;
    bufA[tid.y].z = gid.y;
    InterlockedOr(bufA[tid.y].y, 2);
    InterlockedCompareStore(bufA[tid.y].x, 3, 1);

    // CHECK: Typed resources used in atomic operations must have a scalar element type
    // CHECK: Typed resources used in atomic operations must have a scalar element type
    texA[tid.x] = gid.x;
    texA[tid.y].z = gid.y;
    InterlockedOr(texA[tid.y].y, 2);
    InterlockedCompareStore(texA[tid.y].x, 3, 1);
}
