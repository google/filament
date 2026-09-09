// RUN: %if dxil-1-8  \
// RUN:   %{          \
// RUN: %dxc -T lib_6_8 %s | FileCheck %s --check-prefixes=MD \
// RUN:   %}

// RUN: %if dxil-1-8  \
// RUN:   %{          \
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s --check-prefixes=MD \
// RUN:   %}

// RUN: %if dxil-1-8  \
// RUN:   %{          \
// RUN: %dxc -T lib_6_8 -Zi %s | FileCheck %s --check-prefixes=MD \
// RUN:   %}

// RUN: %if dxil-1-8  \
// RUN:   %{          \
// RUN: %dxc -T lib_6_8 -fcgl %s | FileCheck %s --check-prefix=FCGLMD \
// RUN:   %}

// Verify correct metadata annotations for different node shader input and outputs.

template<typename T>
void foo(T t, out T t2) {
  t2 = t;
}

template<typename T>
T bar(T t) {
  T t2;
  foo(t, t2);
  return t2;
}

template<typename T>
T wrapper(T t) {
  return bar(t);
}


RWBuffer<uint> buf0;

struct RECORD
{
  half h;
  uint b;
  uint ival;
  float fval;
};


struct RECORD1
{
  uint value;
  uint value2;
};

//  DispatchNodeInputRecord

// FCGLMD-DAG:  !{void (%"struct.DispatchNodeInputRecord<RECORD>"*)* @node_DispatchNodeInputRecord, i32 15, i32 1024, i32 1, i32 1, i32 1, i1 false, !"node_DispatchNodeInputRecord", i32 0, !"", i32 0, i32 -1, i32 64, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 97, i32 0, i32 16, i32 0, i32 0, i32 0, i32 4}

// MD-DAG: !{void ()* @node_DispatchNodeInputRecord, !"node_DispatchNodeInputRecord", null, null, ![[DispatchNodeInput:[0-9]+]]}
// MD-DAG: ![[DispatchNodeInput]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD-DAG: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD-DAG: ![[EntryInputs0]] = !{i32 1, i32 97, i32 2, ![[EntryInputs0Record:[0-9]+]]}
// MD-DAG: ![[EntryInputs0Record]] = !{i32 0, i32 16, i32 2, i32 4}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node_DispatchNodeInputRecord(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = wrapper(input).Get().h;
}

//  EmptyNodeInput

// FCGLMD-DAG: !{void (%struct.EmptyNodeInput*)* @node_EmptyNodeInput, i32 15, i32 2, i32 1, i32 1, i32 2, i1 true, !"node_EmptyNodeInput", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 9, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}

// MD-DAG: !{void ()* @node_EmptyNodeInput, !"node_EmptyNodeInput", null, null, ![[EmptyNodeInput:[0-9]+]]}
// MD-DAG: ![[EmptyNodeInput]] = !{i32 8, i32 15, i32 13, i32 2, i32 14, i1 true, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD-DAG: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD-DAG: ![[EntryInputs0]] = !{i32 1, i32 9}

[Shader("node")]
[NodeLaunch("coalescing")]
[NodeIsProgramEntry]
[NumThreads(2,1,1)]
void node_EmptyNodeInput(EmptyNodeInput input)
{
  buf0[0] = wrapper(input).Count();
}


//  EmptyNodeOutput

// FCGLMD-DAG: !{void (%struct.EmptyNodeOutput*)* @node_EmptyNodeOutput, i32 15, i32 1, i32 1, i32 1, i32 1, i1 false, !"node_EmptyNodeOutput", i32 0, !"", i32 0, i32 -1, i32 1, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 10, i32 0, i32 0, i32 0, i32 0, !"loadStressChild", i32 0, i32 0, i32 -1, i32 0, i1 false, i32 0}

// MD: !{void ()* @node_EmptyNodeOutput, !"node_EmptyNodeOutput", null, null, ![[EmptyNodeOutput:[0-9]+]]}
// MD: ![[EmptyNodeOutput]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[EntryOutputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryOutputs]] = !{![[EntryOutputs0:[0-9]+]]}
// MD: ![[EntryOutputs0]] = !{i32 1, i32 10, i32 3, i32 0, i32 0, ![[EntryOutputs0MaxRecords:[0-9]+]]}
// MD: ![[EntryOutputs0MaxRecords]] = !{!"loadStressChild", i32 0}
void loadStressEmptyRecWorker(
EmptyNodeOutput outputNode)
{
	outputNode.GroupIncrementOutputCount(1);
}

[Shader("node")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_EmptyNodeOutput(
	[MaxOutputRecords(1)] EmptyNodeOutput loadStressChild
)
{
	loadStressEmptyRecWorker(wrapper(loadStressChild));
}


//  EmptyNodeOutputArray

// FCGLMD-DAG: !{void (%struct.EmptyNodeOutputArray*)* @node_EmptyNodeOutputArray, i32 15, i32 128, i32 1, i32 1, i32 1, i1 false, !"node_EmptyNodeOutputArray", i32 0, !"", i32 0, i32 -1, i32 1, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 26, i32 0, i32 0, i32 0, i32 0, !"EmptyOutputArray", i32 0, i32 64, i32 -1, i32 128, i1 false, i32 0}

// MD: !{void ()* @node_EmptyNodeOutputArray, !"node_EmptyNodeOutputArray", null, null, ![[EmptyNodeOutputArray:[0-9]+]]}
// MD: ![[EmptyNodeOutputArray]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[EntryOutputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryOutputs]] = !{![[EntryOutputs0:[0-9]+]]}
// MD: ![[EntryOutputs0]] = !{i32 1, i32 26, i32 3, i32 64, i32 5, i32 128, i32 0, ![[EntryOutputs0MaxRecords:[0-9]+]]}
// MD: ![[EntryOutputs0MaxRecords]] = !{!"EmptyOutputArray", i32 0}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_EmptyNodeOutputArray(
    [NodeArraySize(128)] [MaxRecords(64)] EmptyNodeOutputArray EmptyOutputArray
)
{
	bool b = wrapper(EmptyOutputArray)[1].IsValid();
	if (b) {
        wrapper(EmptyOutputArray)[1].GroupIncrementOutputCount(10);
	}
}

//  GroupNodeInputRecords

// FCGLMD-DAG: !{void (%"struct.GroupNodeInputRecords<RECORD>"*)* @node_GroupNodeInputRecords, i32 15, i32 1024, i32 1, i32 1, i32 2, i1 true, !"node_GroupNodeInputRecords", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 65, i32 256, i32 16, i32 0, i32 0, i32 0, i32 4}


// MD: !{void ()* @node_GroupNodeInputRecords, !"node_GroupNodeInputRecords", null, null, ![[GroupNodeInputRecords:[0-9]+]]}
// MD: ![[GroupNodeInputRecords]] = !{i32 8, i32 15, i32 13, i32 2, i32 14, i1 true, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD: ![[EntryInputs0]] = !{i32 1, i32 65, i32 2, ![[EntryInputs0Record]], i32 3, i32 256}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node_GroupNodeInputRecords([MaxRecords(256)] GroupNodeInputRecords<RECORD> inputs)
{
  uint numRecords = wrapper(inputs).Count();
  buf0[0] = numRecords;
}


//  GroupNodeOutputRecords

// FCGLMD-DAG: !{void (%"struct.NodeOutputArray<RECORD1>"*)* @node_GroupNodeOutputRecords, i32 15, i32 128, i32 1, i32 1, i32 1, i1 false, !"node_GroupNodeOutputRecords", i32 0, !"", i32 0, i32 -1, i32 1, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 22, i32 8, i32 0, i32 0, i32 0, !"OutputArray", i32 0, i32 64, i32 -1, i32 128, i1 false, i32 4}

// MD: !{void ()* @node_GroupNodeOutputRecords, !"node_GroupNodeOutputRecords", null, null, ![[GroupNodeOutputRecords:[0-9]+]]}
// MD: ![[GroupNodeOutputRecords]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[EntryOutputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryOutputs]] = !{![[EntryOutputs0:[0-9]+]]}
// MD: ![[EntryOutputs0]] = !{i32 1, i32 22, i32 2, ![[RecordType1:[0-9]+]], i32 3, i32 64, i32 5, i32 128, i32 0, ![[EntryOutputs0MaxRecords:[0-9]+]]}
// MD: ![[RecordType1]] = !{i32 0, i32 8, i32 2, i32 4}
// MD: ![[EntryOutputs0MaxRecords]] = !{!"OutputArray", i32 0}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_GroupNodeOutputRecords(
    [NodeArraySize(128)] [MaxRecords(64)] NodeOutputArray<RECORD1> OutputArray
)
{
	bool b = OutputArray[1].IsValid();
	if (b) {
		GroupNodeOutputRecords<RECORD1> outRec = OutputArray[1].GetGroupNodeOutputRecords(2);
        wrapper(outRec).OutputComplete();
	}
}


//  NodeOutput

// FCGLMD-DAG: !{void (%"struct.NodeOutput<RECORD>"*)* @node_NodeOutput, i32 15, i32 1024, i32 1, i32 1, i32 1, i1 false, !"node_NodeOutput", i32 0, !"", i32 0, i32 -1, i32 32, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 6, i32 16, i32 0, i32 0, i32 0, !"output3", i32 0, i32 0, i32 -1, i32 0, i1 false, i32 4}

// MD: !{void ()* @node_NodeOutput, !"node_NodeOutput", null, null, ![[NodeOutput:[0-9]+]]}
// MD: ![[NodeOutput]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[EntryOutputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryOutputs]] = !{![[EntryOutputs0:[0-9]+]]}
// MD: ![[EntryOutputs0]] = !{i32 1, i32 6, i32 2, ![[EntryInputs0Record]], i32 3, i32 0, i32 0, ![[EntryOutputs0MaxRecords:[0-9]+]]}
// MD: ![[EntryOutputs0MaxRecords]] = !{!"output3", i32 0}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(32,1,1)]
[NodeLaunch("broadcasting")]
void node_NodeOutput(NodeOutput<RECORD> output3)
{
  GroupNodeOutputRecords<RECORD> outrec = wrapper(output3).GetGroupNodeOutputRecords(1);
  InterlockedCompareStoreFloatBitwise(outrec.Get().fval, 0.0, 123.45);

  InterlockedCompareStore(outrec.Get().ival, 111, 222);
  InterlockedAdd(outrec.Get().ival, 333);
}


//  NodeOutputArray

// FCGLMD-DAG: !{void (%"struct.NodeOutputArray<RECORD1>"*)* @node_NodeOutputArray, i32 15, i32 1, i32 1, i32 1, i32 1, i1 false, !"node_NodeOutputArray", i32 0, !"", i32 0, i32 -1, i32 1, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 22, i32 8, i32 0, i32 0, i32 0, !"OutputArray_1_0", i32 0, i32 31, i32 -1, i32 129, i1 true, i32 4}


// MD: !{void ()* @node_NodeOutputArray, !"node_NodeOutputArray", null, null, ![[NodeOutputArray:[0-9]+]]}
// MD: ![[NodeOutputArray]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[NodeOutputArrayEntryOutputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[NodeOutputArrayEntryOutputs]] = !{![[EntryOutputs0:[0-9]+]]}
// MD: ![[EntryOutputs0]] = !{i32 1, i32 22, i32 2, ![[RecordType1]], i32 3, i32 31, i32 5, i32 129, i32 6, i1 true, i32 0, ![[EntryOutputs0MaxRecords:[0-9]+]]}
// MD: ![[EntryOutputs0MaxRecords]] = !{!"OutputArray_1_0", i32 0}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_NodeOutputArray(
    [AllowSparseNodes] [NodeArraySize(129)] [MaxRecords(31)]
    NodeOutputArray<RECORD1> OutputArray_1_0) {
  ThreadNodeOutputRecords<RECORD1> outRec = wrapper(OutputArray_1_0)[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}


//  RWDispatchNodeInputRecord

// FCGLMD-DAG: !{void (%"struct.RWDispatchNodeInputRecord<RECORD>"*)* @node_RWDispatchNodeInputRecord, i32 15, i32 1024, i32 1, i32 1, i32 1, i1 false, !"node_RWDispatchNodeInputRecord", i32 0, !"", i32 0, i32 -1, i32 16, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 101, i32 0, i32 16, i32 0, i32 0, i32 0, i32 4}


// MD: !{void ()* @node_RWDispatchNodeInputRecord, !"node_RWDispatchNodeInputRecord", null, null, ![[RWDispatchNodeInputRecord:[0-9]+]]}
// MD: ![[RWDispatchNodeInputRecord]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD: ![[EntryInputs0]] = !{i32 1, i32 101, i32 2, ![[EntryInputs0Record]]}

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(16,1,1)]
[NodeLaunch("broadcasting")]
void node_RWDispatchNodeInputRecord(RWDispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = wrapper(input).Get().b;
}



//  RWGroupNodeInputRecords

// FCGLMD-DAG: !{void (%"struct.RWGroupNodeInputRecords<RECORD2>"*)* @node_RWGroupNodeInputRecords, i32 15, i32 1, i32 1, i32 1, i32 2, i1 false, !"node_RWGroupNodeInputRecords", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 69, i32 4, i32 48, i32 0, i32 0, i32 0, i32 4}


// MD: !{void ()* @node_RWGroupNodeInputRecords, !"node_RWGroupNodeInputRecords", null, null, ![[RWGroupNodeInputRecords:[0-9]+]]}
// MD: ![[RWGroupNodeInputRecords]] = !{i32 8, i32 15, i32 13, i32 2, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD: ![[EntryInputs0]] = !{i32 1, i32 69, i32 2, ![[RecordType2:[0-9]+]], i32 3, i32 4}
// MD: ![[RecordType2]] = !{i32 0, i32 48, i32 2, i32 4}

struct RECORD2
{
  row_major float2x2 m0;
  row_major float2x2 m1;
  column_major float2x2 m2;
};

[Shader("node")]
[NumThreads(1,1,1)]
[NodeLaunch("coalescing")]
void node_RWGroupNodeInputRecords([MaxRecords(4)] RWGroupNodeInputRecords<RECORD2> input2)
{

  wrapper(input2)[0].m1 = 111;

  wrapper(input2)[1].m2 = wrapper(input2)[1].m0;
}

//  RWThreadNodeInputRecord

// FCGLMD-DAG: !{void (%"struct.RWThreadNodeInputRecord<RECORD>"*)* @node_RWThreadNodeInputRecord, i32 15, i32 1, i32 1, i32 1, i32 3, i1 false, !"node_RWThreadNodeInputRecord", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 37, i32 0, i32 16, i32 0, i32 0, i32 0, i32 4}

// MD: !{void ()* @node_RWThreadNodeInputRecord, !"node_RWThreadNodeInputRecord", null, null, ![[RWThreadNodeInputRecord:[0-9]+]]}
// MD: ![[RWThreadNodeInputRecord]] = !{i32 8, i32 15, i32 13, i32 3, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD: ![[EntryInputs0]] = !{i32 1, i32 37, i32 2, ![[EntryInputs0Record]]}

[Shader("node")]
[NodeLaunch("thread")]
void node_RWThreadNodeInputRecord(RWThreadNodeInputRecord<RECORD> input)
{
   Barrier(wrapper(input), 0);
}

//  ThreadNodeInputRecord

// FCGLMD-DAG: !{void (%"struct.ThreadNodeInputRecord<RECORD>"*)* @node_ThreadNodeInputRecord, i32 15, i32 1, i32 1, i32 1, i32 3, i1 false, !"node_ThreadNodeInputRecord", i32 0, !"", i32 0, i32 -1, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 33, i32 0, i32 16, i32 0, i32 0, i32 0, i32 4}

// MD: !{void ()* @node_ThreadNodeInputRecord, !"node_ThreadNodeInputRecord", null, null, ![[ThreadNodeInputRecord:[0-9]+]]}
// MD: ![[ThreadNodeInputRecord]] = !{i32 8, i32 15, i32 13, i32 3, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 20, ![[EntryInputs:[0-9]+]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}
// MD: ![[EntryInputs]] = !{![[EntryInputs0:[0-9]+]]}
// MD: ![[EntryInputs0]] = !{i32 1, i32 33, i32 2, ![[EntryInputs0Record]]}

[Shader("node")]
[NodeLaunch("thread")]
void node_ThreadNodeInputRecord(ThreadNodeInputRecord<RECORD> input)
{
   Barrier(wrapper(input), 0);
}


//  ThreadNodeOutputRecords

// FCGLMD-DAG: !{void (%"struct.NodeOutputArray<RECORD1>"*)* @node_ThreadNodeOutputRecords, i32 15, i32 1, i32 1, i32 1, i32 1, i1 false, !"node_ThreadNodeOutputRecords", i32 0, !"", i32 0, i32 -1, i32 1, i32 1, i32 1, i32 0, i32 0, i32 0, i32 0, i32 22, i32 8, i32 0, i32 0, i32 0, !"OutputArray_1_0", i32 0, i32 31, i32 -1, i32 129, i1 true, i32 4}

// MD: !{void ()* @node_ThreadNodeOutputRecords, !"node_ThreadNodeOutputRecords", null, null, ![[ThreadNodeOutputRecords:[0-9]+]]}
// MD: ![[ThreadNodeOutputRecords]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, !{{[0-9]+}}, i32 21, ![[NodeOutputArrayEntryOutputs]], i32 4, !{{[0-9]+}}, i32 5, !{{[0-9]+}}}


[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_ThreadNodeOutputRecords(
    [AllowSparseNodes] [NodeArraySize(129)] [MaxRecords(31)]
    NodeOutputArray<RECORD1> OutputArray_1_0) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_0[1].GetThreadNodeOutputRecords(2);
  wrapper(outRec).OutputComplete();
}



