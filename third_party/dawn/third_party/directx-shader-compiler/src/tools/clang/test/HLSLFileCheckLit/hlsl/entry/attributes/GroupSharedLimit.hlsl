// REQUIRES: dxil-1-10

// PASS: usage <= default (no limit)
// RUN: %dxc -E main -T cs_6_10 -DGSM_DWORDS=8192 %s | FileCheck %s

// PASS: default < usage <= limit
// RUN: %dxc -E main -T cs_6_10 -DGSM_DWORDS=9216 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=36864 %s | FileCheck %s

// PASS: no usage, limit=0 (edge case)
// RUN: %dxc -E main -T cs_6_10 -DNO_GSM -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s | FileCheck %s

// PASS: limit == usage < default
// RUN: %dxc -E main -T cs_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=16384 %s | FileCheck %s

// CHECK: @main

// FAIL: limit < usage < default
// RUN: not %dxc -E main -T cs_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=8192 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL1
// CHECK-FAIL1: Total Thread Group Shared Memory used by 'main' is 16384, exceeding explicit limit: 8192.

// FAIL: limit=0 < usage < default (edge case)
// RUN: not %dxc -E main -T cs_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL2
// CHECK-FAIL2: Total Thread Group Shared Memory used by 'main' is 16384, exceeding explicit limit: 0.

#define THREAD_GROUP_SIZE_X 1024

#ifndef NO_GSM
#ifndef GSM_DWORDS
#define GSM_DWORDS 8192
#endif
groupshared uint g_testBuffer[GSM_DWORDS];
#endif

RWStructuredBuffer <uint> g_output : register(u0);

#ifdef USE_GROUP_SHARED_LIMIT
[GroupSharedLimit(LIMIT_BYTES)]
#endif
[numthreads(THREAD_GROUP_SIZE_X, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
#ifndef NO_GSM
    uint iterations = GSM_DWORDS / THREAD_GROUP_SIZE_X;
    
    for (uint i = 0; i < iterations; i++)
    {
        uint index = DTid.x + i * THREAD_GROUP_SIZE_X;
        g_testBuffer[index] = index;
    }
    
    // synchronize all threads in the group
    GroupMemoryBarrierWithGroupSync();

    // write the shared data to the output buffer
    for (uint j = 0; j < iterations; j++)
    {
        uint index = DTid.x + j * THREAD_GROUP_SIZE_X;
        g_output[index] = g_testBuffer[index];
    }
#endif
}
