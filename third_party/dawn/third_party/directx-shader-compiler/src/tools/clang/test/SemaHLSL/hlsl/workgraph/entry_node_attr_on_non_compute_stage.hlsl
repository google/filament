// RUN: %dxc -T ps_6_6 -E main -verify  %s

struct loadStressRecord {
    uint x : SV_DispatchGrid;
};

void loadStressWorker(NodeOutput<loadStressRecord> x) {
    return;
}

[Shader("node")] // expected-error{{entry type 'pixel' from profile 'ps_6_6' conflicts with shader attribute type 'node' on entry function 'main'.}}
[NodeMaxDispatchGrid(3, 1, 1)]
[NumThreads(16, 1, 1)]
void main(    
    DispatchNodeInputRecord<loadStressRecord> input,
    [MaxRecords(16)] NodeOutput<loadStressRecord> loadStressChild
)
{
    loadStressWorker(loadStressChild);
}
