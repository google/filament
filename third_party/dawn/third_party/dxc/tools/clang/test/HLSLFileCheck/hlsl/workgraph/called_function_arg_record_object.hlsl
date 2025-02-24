// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s
//
// Verify that NodeInputRecord can be passed to a called function and used."

struct loadStressRecord
{
    uint  data[29];
    uint3 grid : SV_DispatchGrid;
};

void loadStressWorker(
    inout DispatchNodeInputRecord<loadStressRecord> inputData,
    GroupNodeOutputRecords<loadStressRecord> outRec)
{
    // CHECK: getelementptr inbounds %struct.loadStressRecord.0, %struct.loadStressRecord.0
    uint val =  inputData.Get().data[0]; // problem line

    outRec.Get().data[0] = val + 61;
}

[Shader("node")]
[NodeMaxDispatchGrid(3, 1, 1)]
[NumThreads(16, 1, 1)]
void loadStress_16(DispatchNodeInputRecord<loadStressRecord> inputData,
    [MaxOutputRecordSize(16)] NodeOutput<loadStressRecord> loadStressChild)
{
    loadStressWorker(inputData, loadStressChild.GetGroupNodeOutputRecords(1));
}
