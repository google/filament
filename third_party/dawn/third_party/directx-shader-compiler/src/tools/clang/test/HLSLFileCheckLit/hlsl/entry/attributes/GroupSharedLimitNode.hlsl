// REQUIRES: dxil-1-10

// PASS: usage <= default (no limit)
// RUN: %dxc -T lib_6_10 -DGSM_DWORDS=8192 %s | FileCheck %s

// PASS: default < usage <= limit
// RUN: %dxc -T lib_6_10 -DGSM_DWORDS=9216 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=36864 %s | FileCheck %s

// PASS: no usage, limit=0 (edge case)
// RUN: %dxc -T lib_6_10 -DNO_GSM -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=0 %s | FileCheck %s

// PASS: limit == usage < default
// RUN: %dxc -T lib_6_10 -DGSM_DWORDS=4096 -DUSE_GROUP_SHARED_LIMIT -DLIMIT_BYTES=16384 %s | FileCheck %s

// CHECK: define void @NodeMain()

#define NUM_THREADS 1024

#ifndef NO_GSM
#ifndef GSM_DWORDS
#define GSM_DWORDS 8192
#endif
groupshared uint g_testBuffer[GSM_DWORDS];
#endif

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

#ifndef NO_GSM
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
#endif
}
