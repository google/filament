// REQUIRES: dxil-1-10

// RUN: not %dxc -T lib_6_10 %s 2>&1 | FileCheck %s

// CHECK-DAG: error: Function requires a visible group, but is called from a shader without one.

struct InputRecord {
    uint value;
};

struct OutputRecord {
    uint value;
};

RWStructuredBuffer<uint> output : register(u0);

// Thread launch - no thread group, should FAIL
[Shader("node")]
[NodeLaunch("thread")]
void ThreadNode(
    RWThreadNodeInputRecord<InputRecord> inputData,
    [MaxRecords(1)] NodeOutput<OutputRecord> outputData) {
    uint waveIdx = GetGroupWaveIndex();
    uint waveCount = GetGroupWaveCount();
    ThreadNodeOutputRecords<OutputRecord> outRec = outputData.GetThreadNodeOutputRecords(1);
    outRec.Get().value = waveIdx + waveCount;
    outRec.OutputComplete();
}
