// RUN: %dxc -Tlib_6_8 -verify %s
// ==================================================================
// OutputComplete() is called with unsupported node i/o types
// ==================================================================

struct RECORD {
  int i;
  uint3 grid : SV_DispatchGrid;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(1,1,1)]
void node_dispatchinputrecord(DispatchNodeInputRecord<RECORD> nodeInputRecord)
{
  nodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'DispatchNodeInputRecord<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(1,1,1)]
void node_rwdispatchinputrecord(RWDispatchNodeInputRecord<RECORD> rwNodeInputRecord)
{
  rwNodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'RWDispatchNodeInputRecord<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node_groupinputrecords(GroupNodeInputRecords<RECORD> nodeInputRecord)
{
  nodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'GroupNodeInputRecords<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node_rwgroupinputrecords(RWGroupNodeInputRecords<RECORD> rwNodeInputRecord)
{
  rwNodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'RWGroupNodeInputRecords<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node_threadinputrecord(ThreadNodeInputRecord<RECORD> nodeInputRecord)
{
  nodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'ThreadNodeInputRecord<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node_rwthreadinputrecord(RWThreadNodeInputRecord<RECORD> rwNodeInputRecord)
{
  rwNodeInputRecord.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'RWThreadNodeInputRecord<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node_emptynodeinput([MaxRecords(5)] EmptyNodeInput emptyNodeInput)
{
  emptyNodeInput.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'EmptyNodeInput'}}
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeDispatchGrid(1,1,1)]
void node_nodeoutput(NodeOutput<RECORD> nodeOutput)
{
  nodeOutput.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'NodeOutput<RECORD>'}}
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeDispatchGrid(1,1,1)]
void node127_emptynodeoutput(EmptyNodeOutput emptyNodeOutput)
{
  emptyNodeOutput.OutputComplete(); // expected-error {{no member named 'OutputComplete' in 'EmptyNodeOutput'}}
}
