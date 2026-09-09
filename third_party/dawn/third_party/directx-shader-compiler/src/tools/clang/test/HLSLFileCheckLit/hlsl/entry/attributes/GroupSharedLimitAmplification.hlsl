// REQUIRES: dxil-1-10

// PASS: usage <= default (no limit)
// RUN: %dxc -E main -T as_6_10 -DGSM_DWORDS=8192 %s | FileCheck %s

// PASS: default < usage <= limit
// RUN: %dxc -E main -T as_6_10 -DGSM_DWORDS=9216 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=36864 %s | FileCheck %s

// PASS: no usage, limit=0 (edge case)
// RUN: %dxc -E main -T as_6_10 -DNO_GSM -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s | FileCheck %s

// PASS: limit == usage < default
// RUN: %dxc -E main -T as_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=16384 %s | FileCheck %s

// CHECK: @main

// FAIL: limit < usage < default
// RUN: not %dxc -E main -T as_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=8192 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL1
// CHECK-FAIL1: Total Thread Group Shared Memory used by 'main' is 16384, exceeding explicit limit: 8192.

// FAIL: limit=0 < usage < default (edge case)
// RUN: not %dxc -E main -T as_6_10 -DGSM_DWORDS=1 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL2
// CHECK-FAIL2: Total Thread Group Shared Memory used by 'main' is 4, exceeding explicit limit: 0.

#define NUM_THREADS 32

#ifndef NO_GSM
#ifndef GSM_DWORDS
#define GSM_DWORDS 8192
#endif
groupshared uint g_testBuffer[GSM_DWORDS];
#endif

struct Payload {
    float2 dummy;
    float4 pos;
    float color[2];
};

#ifdef USE_GROUP_SHARED_LIMIT
[GroupSharedLimit(LIMIT_BYTES)]
#endif
[numthreads(NUM_THREADS, 1, 1)]
void main(in uint tig : SV_GroupIndex)
{
#ifndef NO_GSM
    uint iterations = GSM_DWORDS / NUM_THREADS;
    
    for (uint i = 0; i < iterations; i++)
    {
        uint index = tig + i * NUM_THREADS;
        g_testBuffer[index] = index;
    }
    
    // synchronize all threads in the group
    GroupMemoryBarrierWithGroupSync();

    Payload pld;
    pld.dummy = float2(1.0, 2.0);
    pld.pos = float4(g_testBuffer[0], g_testBuffer[1], g_testBuffer[2], g_testBuffer[3]);
    pld.color[0] = 7.0;
    pld.color[1] = 8.0;
    DispatchMesh(NUM_THREADS, 1, 1, pld);
#else
    Payload pld;
    pld.dummy = float2(1.0, 2.0);
    pld.pos = float4(0, 0, 0, 1);
    pld.color[0] = 7.0;
    pld.color[1] = 8.0;
    DispatchMesh(NUM_THREADS, 1, 1, pld);
#endif
}
