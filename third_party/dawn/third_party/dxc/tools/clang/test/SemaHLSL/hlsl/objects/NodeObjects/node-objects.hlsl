// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s --check-prefix=AST

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

//  DispatchNodeInputRecord

// AST: FunctionDecl 0x{{.+}} node_DispatchNodeInputRecord 'void (DispatchNodeInputRecord<RECORD>)'
// AST: |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:35, col:67> col:67 used input 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// call to wrapper
// AST: `-CallExpr 0x{{.+}} <col:13, col:26> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:13> 'DispatchNodeInputRecord<RECORD> (*)(DispatchNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:13> 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'DispatchNodeInputRecord<RECORD> (DispatchNodeInputRecord<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:21> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:21> 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Param]] 'input' 'DispatchNodeInputRecord<RECORD>':'DispatchNodeInputRecord<RECORD>'
// attributes.
// AST: |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: |-HLSLNodeDispatchGridAttr 0x{{.+}} 64 1 1
// AST: |-HLSLNumThreadsAttr 0x{{.+}} 1024 1 1
// AST: `-HLSLShaderAttr 0x{{.+}} "node"


[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(64,1,1)]
[NodeLaunch("broadcasting")]
void node_DispatchNodeInputRecord(DispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = wrapper(input).Get().h;
}


//  RWDispatchNodeInputRecord

// AST: FunctionDecl 0x{{.+}} node_RWDispatchNodeInputRecord 'void (RWDispatchNodeInputRecord<RECORD>)'
// AST: ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:37, col:71> col:71 used input 'RWDispatchNodeInputRecord<RECORD>':'RWDispatchNodeInputRecord<RECORD>'
// call to wrapper
// AST:  `-CallExpr 0x{{.+}} <col:13, col:26> 'RWDispatchNodeInputRecord<RECORD>':'RWDispatchNodeInputRecord<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:13> 'RWDispatchNodeInputRecord<RECORD> (*)(RWDispatchNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:13> 'RWDispatchNodeInputRecord<RECORD> (RWDispatchNodeInputRecord<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'RWDispatchNodeInputRecord<RECORD> (RWDispatchNodeInputRecord<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:21> 'RWDispatchNodeInputRecord<RECORD>':'RWDispatchNodeInputRecord<RECORD>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:21> 'RWDispatchNodeInputRecord<RECORD>':'RWDispatchNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Param]] 'input' 'RWDispatchNodeInputRecord<RECORD>':'RWDispatchNodeInputRecord<RECORD>'
// attributes.
// AST: |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: |-HLSLNodeDispatchGridAttr 0x{{.+}} 16 1 1
// AST: |-HLSLNumThreadsAttr 0x{{.+}} 1024 1 1
// AST: `-HLSLShaderAttr 0x{{.+}} "node"

[Shader("node")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(16,1,1)]
[NodeLaunch("broadcasting")]
void node_RWDispatchNodeInputRecord(RWDispatchNodeInputRecord<RECORD> input)
{
  buf0[0] = wrapper(input).Get().b;
}


//  GroupNodeInputRecords

// AST: FunctionDecl 0x{{.+}} node_GroupNodeInputRecords 'void (GroupNodeInputRecords<RECORD>)'
// AST: |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:51, col:81> col:81 used inputs 'GroupNodeInputRecords<RECORD>':'GroupNodeInputRecords<RECORD>'
// AST: | `-HLSLMaxRecordsAttr 0x{{.+}} <col:34, col:48> 256
// call to wrapper
// AST: `-CallExpr 0x{{.+}} <col:21, col:35> 'GroupNodeInputRecords<RECORD>':'GroupNodeInputRecords<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:21> 'GroupNodeInputRecords<RECORD> (*)(GroupNodeInputRecords<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:21> 'GroupNodeInputRecords<RECORD> (GroupNodeInputRecords<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'GroupNodeInputRecords<RECORD> (GroupNodeInputRecords<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:29> 'GroupNodeInputRecords<RECORD>':'GroupNodeInputRecords<RECORD>' <LValueToRValue>
// AST:  `-DeclRefExpr 0x{{.+}} <col:29> 'GroupNodeInputRecords<RECORD>':'GroupNodeInputRecords<RECORD>' lvalue ParmVar 0x[[Param]] 'inputs' 'GroupNodeInputRecords<RECORD>':'GroupNodeInputRecords<RECORD>'
// attributes.
// AST: |-HLSLNodeIsProgramEntryAttr 0x{{.+}}
// AST: |-HLSLNumThreadsAttr 0x{{.+}} 1024 1 1
// AST: |-HLSLNodeLaunchAttr 0x{{.+}} "coalescing"
// AST: `-HLSLShaderAttr 0x{{.+}} "node"

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node_GroupNodeInputRecords([MaxRecords(256)] GroupNodeInputRecords<RECORD> inputs)
{
  uint numRecords = wrapper(inputs).Count();
  buf0[0] = numRecords;
}

//  RWGroupNodeInputRecords

// AST: FunctionDecl 0x{{.+}} node_RWGroupNodeInputRecords 'void (RWGroupNodeInputRecords<RECORD2>)'
// AST: |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:51, col:84> col:84 used input2 'RWGroupNodeInputRecords<RECORD2>':'RWGroupNodeInputRecords<RECORD2>'
// AST: | `-HLSLMaxRecordsAttr 0x{{.+}} <col:36, col:48> 4
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:3, col:17> 'RWGroupNodeInputRecords<RECORD2>':'RWGroupNodeInputRecords<RECORD2>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:3> 'RWGroupNodeInputRecords<RECORD2> (*)(RWGroupNodeInputRecords<RECORD2>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:3> 'RWGroupNodeInputRecords<RECORD2> (RWGroupNodeInputRecords<RECORD2>)' lvalue Function 0x{{.+}} 'wrapper' 'RWGroupNodeInputRecords<RECORD2> (RWGroupNodeInputRecords<RECORD2>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: | `-ImplicitCastExpr 0x{{.+}} <col:11> 'RWGroupNodeInputRecords<RECORD2>':'RWGroupNodeInputRecords<RECORD2>' <LValueToRValue>
// AST: |   `-DeclRefExpr 0x{{.+}} <col:11> 'RWGroupNodeInputRecords<RECORD2>':'RWGroupNodeInputRecords<RECORD2>' lvalue ParmVar 0x[[Param]] 'input2' 'RWGroupNodeInputRecords<RECORD2>':'RWGroupNodeInputRecords<RECORD2>'
// attributes.
// AST: |-HLSLNodeLaunchAttr 0x{{.+}} "coalescing"
// AST: |-HLSLNumThreadsAttr 0x{{.+}} 1 1 1
// AST: `-HLSLShaderAttr 0x{{.+}} "node"

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

//  ThreadNodeInputRecord

// AST: FunctionDecl 0x{{.+}} node_ThreadNodeInputRecord 'void (ThreadNodeInputRecord<RECORD>)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:33, col:63> col:63 used input 'ThreadNodeInputRecord<RECORD>':'ThreadNodeInputRecord<RECORD>'
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:12, col:25> 'ThreadNodeInputRecord<RECORD>':'ThreadNodeInputRecord<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:12> 'ThreadNodeInputRecord<RECORD> (*)(ThreadNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:12> 'ThreadNodeInputRecord<RECORD> (ThreadNodeInputRecord<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'ThreadNodeInputRecord<RECORD> (ThreadNodeInputRecord<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST:  `-ImplicitCastExpr 0x{{.+}} <col:20> 'ThreadNodeInputRecord<RECORD>':'ThreadNodeInputRecord<RECORD>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:20> 'ThreadNodeInputRecord<RECORD>':'ThreadNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Param]] 'input' 'ThreadNodeInputRecord<RECORD>':'ThreadNodeInputRecord<RECORD>'
// attributes.
// AST: |-HLSLNodeLaunchAttr 0x{{.+}} "thread"
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"

[Shader("node")]
[NodeLaunch("thread")]
void node_ThreadNodeInputRecord(ThreadNodeInputRecord<RECORD> input)
{
   Barrier(wrapper(input), 0);
}

//  RWThreadNodeInputRecord

// AST: FunctionDecl 0x{{.+}} node_RWThreadNodeInputRecord 'void (RWThreadNodeInputRecord<RECORD>)'
// AST: |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:35, col:67> col:67 used input 'RWThreadNodeInputRecord<RECORD>':'RWThreadNodeInputRecord<RECORD>'
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:12, col:25> 'RWThreadNodeInputRecord<RECORD>':'RWThreadNodeInputRecord<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:12> 'RWThreadNodeInputRecord<RECORD> (*)(RWThreadNodeInputRecord<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:12> 'RWThreadNodeInputRecord<RECORD> (RWThreadNodeInputRecord<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'RWThreadNodeInputRecord<RECORD> (RWThreadNodeInputRecord<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST:  `-ImplicitCastExpr 0x{{.+}} <col:20> 'RWThreadNodeInputRecord<RECORD>':'RWThreadNodeInputRecord<RECORD>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:20> 'RWThreadNodeInputRecord<RECORD>':'RWThreadNodeInputRecord<RECORD>' lvalue ParmVar 0x[[Param]] 'input' 'RWThreadNodeInputRecord<RECORD>':'RWThreadNodeInputRecord<RECORD>'
// attributes.
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "thread"
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"
[Shader("node")]
[NodeLaunch("thread")]
void node_RWThreadNodeInputRecord(RWThreadNodeInputRecord<RECORD> input)
{
   Barrier(wrapper(input), 0);
}
//  EmptyNodeInput
// AST: FunctionDecl 0x{{.+}} node_EmptyNodeInput 'void (EmptyNodeInput)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:26, col:41> col:41 used input 'EmptyNodeInput'
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:13, col:26> 'EmptyNodeInput':'EmptyNodeInput'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:13> 'EmptyNodeInput (*)(EmptyNodeInput)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:13> 'EmptyNodeInput (EmptyNodeInput)' lvalue Function 0x{{.+}} 'wrapper' 'EmptyNodeInput (EmptyNodeInput)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:21> 'EmptyNodeInput' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:21> 'EmptyNodeInput' lvalue ParmVar 0x[[Param]] 'input' 'EmptyNodeInput'
// attributes.
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 2 1 1
// AST: | |-HLSLNodeIsProgramEntryAttr
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "coalescing"
// AST: | `-HLSLShaderAttr 0x{{.+}}> "node"

[Shader("node")]
[NodeLaunch("coalescing")]
[NodeIsProgramEntry]
[NumThreads(2,1,1)]
void node_EmptyNodeInput(EmptyNodeInput input)
{
  buf0[0] = wrapper(input).Count();
}


//  NodeOutput

// AST: FunctionDecl 0x{{.+}} node_NodeOutput 'void (NodeOutput<RECORD>)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] <col:22, col:41> col:41 used output3 'NodeOutput<RECORD>':'NodeOutput<RECORD>'
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:43, col:58> 'NodeOutput<RECORD>':'NodeOutput<RECORD>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:43> 'NodeOutput<RECORD> (*)(NodeOutput<RECORD>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:43> 'NodeOutput<RECORD> (NodeOutput<RECORD>)' lvalue Function 0x{{.+}} 'wrapper' 'NodeOutput<RECORD> (NodeOutput<RECORD>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST:  `-ImplicitCastExpr 0x{{.+}} <col:51> 'NodeOutput<RECORD>':'NodeOutput<RECORD>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:51> 'NodeOutput<RECORD>':'NodeOutput<RECORD>' lvalue ParmVar 0x[[Param]] 'output3' 'NodeOutput<RECORD>':'NodeOutput<RECORD>'
// attributes.
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: | |-HLSLNodeDispatchGridAttr 0x{{.+}} 32 1 1
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 1024 1 1
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"

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

//  EmptyNodeOutput
// AST: FunctionDecl 0x{{.+}} node_EmptyNodeOutput 'void (EmptyNodeOutput)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] {{.+}} col:35 used loadStressChild 'EmptyNodeOutput'
// AST: | | `-HLSLMaxRecordsAttr 0x{{.+}} 12
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:27, col:50> 'EmptyNodeOutput':'EmptyNodeOutput'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:27> 'EmptyNodeOutput (*)(EmptyNodeOutput)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:27> 'EmptyNodeOutput (EmptyNodeOutput)' lvalue Function 0x{{.+}} 'wrapper' 'EmptyNodeOutput (EmptyNodeOutput)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:35> 'EmptyNodeOutput' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:35> 'EmptyNodeOutput' lvalue ParmVar 0x[[Param]] 'loadStressChild' 'EmptyNodeOutput'
// attributes.
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 1 1 1
// AST: | |-HLSLNodeDispatchGridAttr 0x{{.+}} 1 1 1
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"
void loadStressEmptyRecWorker(
EmptyNodeOutput outputNode)
{
	outputNode.GroupIncrementOutputCount(1);
}

[Shader("node")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_EmptyNodeOutput(
	[MaxRecords(12)] EmptyNodeOutput loadStressChild
)
{
	loadStressEmptyRecWorker(wrapper(loadStressChild));
}

//  NodeOutputArray
// AST: FunctionDecl 0x{{.+}} node_NodeOutputArray 'void (NodeOutputArray<RECORD1>)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] {{.+}} col:30 used OutputArray_1_0 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST: | | |-HLSLMaxRecordsAttr 0x{{.+}} 31
// AST: | | |-HLSLNodeArraySizeAttr 0x{{.+}} 129
// AST: | | `-HLSLAllowSparseNodesAttr 0x{{.+}} <col:6>
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:45, col:68> 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:45> 'NodeOutputArray<RECORD1> (*)(NodeOutputArray<RECORD1>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:45> 'NodeOutputArray<RECORD1> (NodeOutputArray<RECORD1>)' lvalue Function 0x{{.+}} 'wrapper' 'NodeOutputArray<RECORD1> (NodeOutputArray<RECORD1>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:53> 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:53> 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>' lvalue ParmVar 0x[[Param]] 'OutputArray_1_0' 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// attributes.
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 1 1 1
// AST: | |-HLSLNodeDispatchGridAttr 0x{{.+}} 1 1 1
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"

struct RECORD1
{
  uint value;
  uint value2;
};

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

//  EmptyNodeOutputArray
// AST: FunctionDecl 0x{{.+}} node_EmptyNodeOutputArray 'void (EmptyNodeOutputArray)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] {{.+}} col:64 used EmptyOutputArray 'EmptyNodeOutputArray'
// AST: | | |-HLSLMaxRecordsAttr 0x{{.+}} <col:27, col:40> 64
// AST: | | `-HLSLNodeArraySizeAttr 0x{{.+}} <col:6, col:23> 128
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:11, col:35> 'EmptyNodeOutputArray':'EmptyNodeOutputArray'
// AST: -ImplicitCastExpr 0x{{.+}} <col:11> 'EmptyNodeOutputArray (*)(EmptyNodeOutputArray)' <FunctionToPointerDecay>
// AST: `-DeclRefExpr 0x{{.+}} <col:11> 'EmptyNodeOutputArray (EmptyNodeOutputArray)' lvalue Function 0x{{.+}} 'wrapper' 'EmptyNodeOutputArray (EmptyNodeOutputArray)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:19> 'EmptyNodeOutputArray' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:19> 'EmptyNodeOutputArray' lvalue ParmVar 0x[[Param]] 'EmptyOutputArray' 'EmptyNodeOutputArray'
// attributes.
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 128 1 1
// AST: | |-HLSLNodeDispatchGridAttr 0x{{.+}} 1 1 1
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"

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

//  GroupNodeOutputRecords
// AST: FunctionDecl 0x{{.+}} node_GroupNodeOutputRecords 'void (NodeOutputArray<RECORD1>)'
// AST: | |-ParmVarDecl 0x[[Param:[0-9a-f]+]] {{.+}} col:68 used OutputArray 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST: | | |-HLSLMaxRecordsAttr 0x{{.+}} <col:27, col:40> 64
// AST: | | `-HLSLNodeArraySizeAttr 0x{{.+}} <col:6, col:23> 128
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:9, col:23> 'GroupNodeOutputRecords<RECORD1>':'GroupNodeOutputRecords<RECORD1>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:9> 'GroupNodeOutputRecords<RECORD1> (*)(GroupNodeOutputRecords<RECORD1>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:9> 'GroupNodeOutputRecords<RECORD1> (GroupNodeOutputRecords<RECORD1>)' lvalue Function 0x{{.+}} 'wrapper' 'GroupNodeOutputRecords<RECORD1> (GroupNodeOutputRecords<RECORD1>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:17> 'GroupNodeOutputRecords<RECORD1>':'GroupNodeOutputRecords<RECORD1>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:17> 'GroupNodeOutputRecords<RECORD1>':'GroupNodeOutputRecords<RECORD1>' lvalue Var 0x{{.+}} 'outRec' 'GroupNodeOutputRecords<RECORD1>':'GroupNodeOutputRecords<RECORD1>'
// attributes.
// AST: | |-HLSLNumThreadsAttr 0x{{.+}} 128 1 1
// AST: | |-HLSLNodeDispatchGridAttr 0x{{.+}} 1 1 1
// AST: | |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST: | `-HLSLShaderAttr 0x{{.+}} "node"
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

//  ThreadNodeOutputRecords
// AST: FunctionDecl 0x{{.+}} node_ThreadNodeOutputRecords 'void (NodeOutputArray<RECORD1>)'
// AST:   |-ParmVarDecl 0x[[Param:[0-9a-f]+]] {{.+}} used OutputArray_1_0 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST:   | |-HLSLMaxRecordsAttr 0x{{.+}} 31
// AST:   | |-HLSLNodeArraySizeAttr 0x{{.+}} <col:25, col:42> 129
// AST:   | `-HLSLAllowSparseNodesAttr 0x{{.+}} <col:6>
// call to wrapper
// AST: CallExpr 0x{{.+}} <col:3, col:17> 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>'
// AST: |-ImplicitCastExpr 0x{{.+}} <col:3> 'ThreadNodeOutputRecords<RECORD1> (*)(ThreadNodeOutputRecords<RECORD1>)' <FunctionToPointerDecay>
// AST: | `-DeclRefExpr 0x{{.+}} <col:3> 'ThreadNodeOutputRecords<RECORD1> (ThreadNodeOutputRecords<RECORD1>)' lvalue Function 0x{{.+}} 'wrapper' 'ThreadNodeOutputRecords<RECORD1> (ThreadNodeOutputRecords<RECORD1>)' (FunctionTemplate 0x{{.+}} 'wrapper')
// AST: `-ImplicitCastExpr 0x{{.+}} <col:11> 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>' <LValueToRValue>
// AST: `-DeclRefExpr 0x{{.+}} <col:11> 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>' lvalue Var 0x{{.+}} 'outRec' 'ThreadNodeOutputRecords<RECORD1>':'ThreadNodeOutputRecords<RECORD1>'
// attributes.
// AST:   |-HLSLNumThreadsAttr 0x{{.+}} 1 1 1
// AST:   |-HLSLNodeDispatchGridAttr 0x{{.+}} 1 1 1
// AST:   |-HLSLNodeLaunchAttr 0x{{.+}} "broadcasting"
// AST:   `-HLSLShaderAttr 0x{{.+}} "node"

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

