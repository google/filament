// REQUIRES: dxil-1-10

// RUN: %dxc -T lib_6_10 %s 2>&1 | FileCheck %s

// CHECK-DAG: define void @BroadcastingNode
// CHECK-DAG: define void @CoalescingNode

struct InputRecord {
    uint value;
};

struct OutputRecord {
    uint value;
};

RWStructuredBuffer<uint> output : register(u0);

// Broadcasting launch - has thread group, should work
[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1,1,1)]
[NumThreads(8,1,1)]
void BroadcastingNode(DispatchNodeInputRecord<InputRecord> inputData) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    output[0] = waveIdx + waveCount + inputData.Get().value;
}

// Coalescing launch - has thread group, should work
[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(8,1,1)]
void CoalescingNode(
    [MaxRecords(8)] GroupNodeInputRecords<InputRecord> inputData,
    [MaxRecords(8)] NodeOutput<OutputRecord> outputData) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    GroupNodeOutputRecords<OutputRecord> outRec = outputData.GetGroupNodeOutputRecords(1);
    outRec.Get().value = waveIdx + waveCount;
    outRec.OutputComplete();
}