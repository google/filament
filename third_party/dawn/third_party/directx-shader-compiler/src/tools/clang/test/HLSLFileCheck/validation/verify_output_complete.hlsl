// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// TBD: Prevent (%dx.types.NodeRecordHandle zeroinitializer) even earlier if possible.
// CHECK: error: Instructions should not read uninitialized value.
// CHECK-NEXT: call %struct.loadStressRecord.0 addrspace(6)* @dx.op.getNodeRecordPtr.struct.loadStressRecord.0(i32 239, %dx.types.NodeRecordHandle zeroinitializer,
#define LOAD_STRESS_MAX_GRID_SIZE 3
#define GROUP_SHARED_SIZE 128

struct loadStressRecord
{
    uint3 grid : SV_DispatchGrid;
    uint  data[3];
};

groupshared uint loadStressTemp[GROUP_SHARED_SIZE];


void loadStressWorker(
    uint threadIndex,
    uint threadGroupSize,
    //inout NodeInputRecord<loadStressRecord> inputData,
    NodeOutput<loadStressRecord> outputNode)
{

    ThreadNodeOutputRecords<loadStressRecord> outRec = outputNode.GetThreadNodeOutputRecords(1);

    outRec.Get().grid = uint3(threadIndex % LOAD_STRESS_MAX_GRID_SIZE + 1, 1, 1);
    for (uint i = 0; i < 3; i++)
    {
        outRec.Get().data[i] = loadStressTemp[i % GROUP_SHARED_SIZE];
    }

    outRec.OutputComplete();
    if (threadIndex % 3) {
        // error: Invalid use of completed record handle.
        // here is where the zeroinitializer is used as a handle value
        outRec.Get().data[0] = 0;
    }
}

#define LOAD_STRESS_THREAD_GROUP_SIZE_16 1

[Shader("node")]
[NodeMaxDispatchGrid(LOAD_STRESS_MAX_GRID_SIZE, 1, 1)]
[NumThreads(LOAD_STRESS_THREAD_GROUP_SIZE_16, 1, 1)]
void loadStress_16 (DispatchNodeInputRecord<loadStressRecord> input,
                    NodeOutput<loadStressRecord> loadStressChild,
                    uint threadIndex : SV_GroupIndex )
{
    loadStressWorker(threadIndex, LOAD_STRESS_THREAD_GROUP_SIZE_16, /*inputData, */loadStressChild);
}
