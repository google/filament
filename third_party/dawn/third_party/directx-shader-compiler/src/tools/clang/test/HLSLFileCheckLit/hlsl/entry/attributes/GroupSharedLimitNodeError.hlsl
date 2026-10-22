// REQUIRES: dxil-1-10

// FAIL: default < limit < usage (36864 < 40960)
// RUN: not %dxc -T lib_6_10 -DGSM_DWORDS=10240 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=36864 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL1
// CHECK-FAIL1: Total Thread Group Shared Memory used by 'NodeMain' is 40960, exceeding explicit limit: 36864.

// FAIL: default < usage (no limit) (32768 < 36864)
// RUN: not %dxc -T lib_6_10 -DGSM_DWORDS=9216 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL2
// CHECK-FAIL2: Total Thread Group Shared Memory used by 'NodeMain' is 36864, exceeding maximum: 32768

// FAIL: limit < usage < default (8192 < 16384 < 32768)
// RUN: not %dxc -T lib_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=8192 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL3
// CHECK-FAIL3: Total Thread Group Shared Memory used by 'NodeMain' is 16384, exceeding explicit limit: 8192.

// FAIL: limit=0 < usage < default (0 < 16384 < 32768) (edge case)
// RUN: not %dxc -T lib_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s 2>&1 | FileCheck %s --check-prefix=CHECK-FAIL4
// CHECK-FAIL4: Total Thread Group Shared Memory used by 'NodeMain' is 16384, exceeding explicit limit: 0.

#define NUM_THREADS 1024

#ifndef GSM_DWORDS
#define GSM_DWORDS 8192
#endif
groupshared uint g_testBuffer[GSM_DWORDS];

RWStructuredBuffer<uint> g_output : register(u0);

struct MY_INPUT_RECORD {
    uint data;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(NUM_THREADS, 1, 1)]
#ifdef USE_GROUP_SHARED_LIMIT
[GroupSharedLimit(LIMIT_BYTES)]
#endif
void NodeMain(DispatchNodeInputRecord<MY_INPUT_RECORD> myInput)
{
    uint tid = myInput.Get().data;
    uint iterations = GSM_DWORDS / NUM_THREADS;
    
    for (uint i = 0; i < iterations; i++)
    {
        uint index = tid + i * NUM_THREADS;
        g_testBuffer[index] = index;
    }
    
    GroupMemoryBarrierWithGroupSync();

    // Write the shared data to the output buffer
    for (uint j = 0; j < iterations; j++)
    {
        uint index = tid + j * NUM_THREADS;
        g_output[index] = g_testBuffer[index];
    }
}
