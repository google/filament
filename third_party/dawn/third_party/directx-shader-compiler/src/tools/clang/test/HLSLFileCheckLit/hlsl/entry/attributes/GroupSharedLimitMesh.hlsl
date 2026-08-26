// REQUIRES: dxil-1-10

// PASS: usage <= default (no limit)
// RUN: %dxc -E main -T ms_6_10 -DGSM_DWORDS=7168 %s | FileCheck %s

// PASS: default < usage <= limit
// RUN: %dxc -E main -T ms_6_10 -DGSM_DWORDS=9216 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=36864 %s | FileCheck %s

// PASS: no usage, limit=0 (edge case)
// RUN: %dxc -E main -T ms_6_10 -DNO_GSM -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s | FileCheck %s

// PASS: limit == usage < default
// RUN: %dxc -E main -T ms_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=16384 %s | FileCheck %s

// CHECK: @main

// FAIL: limit < usage < default
// RUN: not %dxc -E main -T ms_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=8192 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL1
// CHECK-FAIL1: Total Thread Group Shared Memory used by 'main' is 16384, exceeding explicit limit: 8192.

// FAIL: limit=0 < usage < default (edge case)
// RUN: not %dxc -E main -T ms_6_10 -DGSM_DWORDS=1 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL2
// CHECK-FAIL2: Total Thread Group Shared Memory used by 'main' is 4, exceeding explicit limit: 0.

#define NUM_THREADS 32
#define MAX_VERT 32
#define MAX_PRIM 16

#ifndef NO_GSM
#ifndef GSM_DWORDS
#define GSM_DWORDS 7168
#endif
groupshared uint g_testBuffer[GSM_DWORDS];
#endif

struct MeshPerVertex {
    float4 position : SV_Position;
};

struct MeshPerPrimitive {
    float normal : NORMAL;
};

struct MeshPayload {
    float data;
};

#ifdef USE_GROUP_SHARED_LIMIT
[GroupSharedLimit(LIMIT_BYTES)]
#endif
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
            out indices uint3 primIndices[MAX_PRIM],
            out vertices MeshPerVertex verts[MAX_VERT],
            out primitives MeshPerPrimitive prims[MAX_PRIM],
            in payload MeshPayload mpl,
            in uint tig : SV_GroupIndex
         )
{
    SetMeshOutputCounts(MAX_VERT, MAX_PRIM);

#ifndef NO_GSM
    uint iterations = GSM_DWORDS / NUM_THREADS;
    
    for (uint i = 0; i < iterations; i++)
    {
        uint index = tig + i * NUM_THREADS;
        g_testBuffer[index] = index;
    }
    
    // synchronize all threads in the group
    GroupMemoryBarrierWithGroupSync();

    // Use shared memory data to set vertex positions
    if (tig < MAX_VERT)
    {
        verts[tig].position = float4(g_testBuffer[tig], 0, 0, 1);
    }
#else
    if (tig < MAX_VERT)
    {
        verts[tig].position = float4(0, 0, 0, 1);
    }
#endif
    
    if (tig < MAX_PRIM)
    {
        primIndices[tig] = uint3(tig % MAX_VERT, (tig + 1) % MAX_VERT, (tig + 2) % MAX_VERT);
        prims[tig].normal = mpl.data;
    }
}
