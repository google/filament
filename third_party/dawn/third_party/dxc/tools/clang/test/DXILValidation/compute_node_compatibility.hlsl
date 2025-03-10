// ==================================================================
// This HLSL source is used by ValidationTest::ComputeNodeCompatibility,
// which modifies metadata in this compiled library to verify 
// validation rules related to node shader entry metadata.
// The shader is compiled with "-T lib_6_8 -HV 2021"
// ==================================================================

struct RECORD {
  uint a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input) { }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1,1,1)]
void node02(RWDispatchNodeInputRecord<RECORD> input) { }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(3, 1, 1)]
[NumThreads(1,1,1)]
void node03(NodeOutput<RECORD> output) { }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node04() { }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1,1,1)]
void node05() { }
