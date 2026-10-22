// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s -check-prefix=AST
// RUN: %dxc -T lib_6_8 -fcgl %s | FileCheck %s -check-prefix=HLCHECK
// RUN: %dxc -T lib_6_8 %s | %D3DReflect %s | FileCheck %s -check-prefix=REFLECT
// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s -check-prefixes=CHECK,CHECKOD

// Tests for [AllowSparseNodes] and [UnboundedSparseNodes] on NodeOutputArray/EmptyNodeOutputArray

// ==== AST Checks ====

// AST: FunctionDecl {{.*}} node_1_0 'void (NodeOutputArray<RECORD1>)'
// AST-NEXT: ParmVarDecl {{.*}} used OutputArray_1_0 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 31
// AST-NEXT: HLSLNodeArraySizeAttr {{.*}} 129
// AST-NEXT: HLSLAllowSparseNodesAttr

// AST: FunctionDecl {{.*}} node_1_1 'void (NodeOutputArray<RECORD1>)'
// AST-NEXT: ParmVarDecl {{.*}} used OutputArray_1_1 'NodeOutputArray<RECORD1>':'NodeOutputArray<RECORD1>'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 37
// AST-NEXT: HLSLUnboundedSparseNodesAttr

// AST: FunctionDecl {{.*}} node_1_2 'void (NodeOutput<RECORD1>)'
// AST-NEXT: ParmVarDecl {{.*}} used Output_1_2 'NodeOutput<RECORD1>':'NodeOutput<RECORD1>'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 47
// AST-NEXT: HLSLAllowSparseNodesAttr

// AST: FunctionDecl {{.*}} node_2_0 'void (EmptyNodeOutputArray)'
// AST-NEXT: ParmVarDecl {{.*}} used OutputArray_2_0 'EmptyNodeOutputArray'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 41
// AST-NEXT: HLSLNodeArraySizeAttr {{.*}} 131
// AST-NEXT: HLSLAllowSparseNodesAttr

// AST: FunctionDecl {{.*}} node_2_1 'void (EmptyNodeOutputArray)'
// AST-NEXT: ParmVarDecl {{.*}} used OutputArray_2_1 'EmptyNodeOutputArray'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 43
// AST-NEXT: HLSLUnboundedSparseNodesAttr

// AST: FunctionDecl {{.*}} node_2_2 'void (EmptyNodeOutput)'
// AST-NEXT: ParmVarDecl {{.*}} used Output_2_2 'EmptyNodeOutput'
// AST-NEXT: HLSLMaxRecordsAttr {{.*}} 53
// AST-NEXT: HLSLAllowSparseNodesAttr

// ==== -fcgl Metadata Checks ====

// Make sure AllowSparseNodes is true
// Make sure OutputArraySize is -1 for [UnboundedSparseNodes]
// Starting from !"OutputArray_*": OutputID.Name, OutputID.Index, MaxRecords, MaxRecordsSharedWith, OutputArraySize, AllowSparseNodes, Alignment
// HLCHECK-DAG: = !{void (%"struct.NodeOutputArray<RECORD1>"*)* @node_1_0, {{.*}} !"OutputArray_1_0", i32 0, i32 31, i32 -1, i32 129, i1 true, i32 4}
// HLCHECK-DAG: = !{void (%"struct.NodeOutputArray<RECORD1>"*)* @node_1_1, {{.*}} !"OutputArray_1_1", i32 0, i32 37, i32 -1, i32 -1, i1 true, i32 4}
// HLCHECK-DAG: = !{void (%"struct.NodeOutput<RECORD1>"*)* @node_1_2, {{.*}} !"Output_1_2", i32 0, i32 47, i32 -1, i32 0, i1 true, i32 4}
// HLCHECK-DAG: = !{void (%struct.EmptyNodeOutputArray*)* @node_2_0, {{.*}} !"OutputArray_2_0", i32 0, i32 41, i32 -1, i32 131, i1 true, i32 0}
// HLCHECK-DAG: = !{void (%struct.EmptyNodeOutputArray*)* @node_2_1, {{.*}} !"OutputArray_2_1", i32 0, i32 43, i32 -1, i32 -1, i1 true, i32 0}
// HLCHECK-DAG: = !{void (%struct.EmptyNodeOutput*)* @node_2_2, {{.*}} !"Output_2_2", i32 0, i32 53, i32 -1, i32 0, i1 true, i32 0}

// ==== RDAT Checks ====

// REFLECT-LABEL: UnmangledName: "node_1_0"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 22
// REFLECT-DAG: AttribKind: OutputArraySize
// REFLECT-NEXT: OutputArraySize: 129
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 31
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// REFLECT-LABEL: UnmangledName: "node_1_1"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 22
// REFLECT-DAG: AttribKind: OutputArraySize
// REFLECT-NEXT: OutputArraySize: 4294967295
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 37
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// REFLECT-LABEL: UnmangledName: "node_1_2"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 6
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 47
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// REFLECT-LABEL: UnmangledName: "node_2_0"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 26
// REFLECT-DAG: AttribKind: OutputArraySize
// REFLECT-NEXT: OutputArraySize: 131
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 41
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// REFLECT-LABEL: UnmangledName: "node_2_1"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 26
// REFLECT-DAG: AttribKind: OutputArraySize
// REFLECT-NEXT: OutputArraySize: 4294967295
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 43
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// REFLECT-LABEL: UnmangledName: "node_2_2"
// REFLECT: Outputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]>  = {
// REFLECT: IOFlagsAndKind: 10
// REFLECT-DAG: AttribKind: MaxRecords
// REFLECT-NEXT: MaxRecords: 53
// REFLECT-DAG: AttribKind: AllowSparseNodes
// REFLECT-NEXT: AllowSparseNodes: 1

// ==== DXIL Metadata Checks ====

// CHECK: = !{void ()* @node_1_0, !"node_1_0", null, null, ![[EntryAttrs_1_0:[0-9]+]]}
// CHECK: ![[EntryAttrs_1_0]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_1_0:[0-9]+]],
// CHECK: ![[NodeInputs_1_0]] = !{![[NodeInput_1_0:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), NodeArray(16) | ReadWrite(4) | Output(2), ...
// kDxilNodeMaxRecordsTag(3), 31, kDxilNodeOutputArraySizeTag(5), 129, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_1_0]] = !{i32 1, i32 22, {{.*}} i32 3, i32 31, i32 5, i32 129, i32 6, i1 true,

// CHECK: = !{void ()* @node_1_1, !"node_1_1", null, null, ![[EntryAttrs_1_1:[0-9]+]]}
// CHECK: ![[EntryAttrs_1_1]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_1_1:[0-9]+]],
// CHECK: ![[NodeInputs_1_1]] = !{![[NodeInput_1_1:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), NodeArray(16) | ReadWrite(4) | Output(2), ...
// kDxilNodeMaxRecordsTag(3), 37, kDxilNodeOutputArraySizeTag(5), -1, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_1_1]] = !{i32 1, i32 22, {{.*}} i32 3, i32 37, i32 5, i32 -1, i32 6, i1 true,

// CHECK: = !{void ()* @node_1_2, !"node_1_2", null, null, ![[EntryAttrs_1_2:[0-9]+]]}
// CHECK: ![[EntryAttrs_1_2]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_1_2:[0-9]+]],
// CHECK: ![[NodeInputs_1_2]] = !{![[NodeInput_1_2:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), ReadWrite(4) | Output(2), ...
// kDxilNodeMaxRecordsTag(3), 47, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_1_2]] = !{i32 1, i32 6, {{.*}} i32 3, i32 47,{{.*}} i32 6, i1 true,

// CHECK: = !{void ()* @node_2_0, !"node_2_0", null, null, ![[EntryAttrs_2_0:[0-9]+]]}
// CHECK: ![[EntryAttrs_2_0]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_2_0:[0-9]+]],
// CHECK: ![[NodeInputs_2_0]] = !{![[NodeInput_2_0:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), NodeArray(16) | EmptyRecord(8) | Output(2),
// kDxilNodeMaxRecordsTag(3), 41, kDxilNodeOutputArraySizeTag(5), 131, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_2_0]] = !{i32 1, i32 26, i32 3, i32 41, i32 5, i32 131, i32 6, i1 true,

// CHECK: = !{void ()* @node_2_1, !"node_2_1", null, null, ![[EntryAttrs_2_1:[0-9]+]]}
// CHECK: ![[EntryAttrs_2_1]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_2_1:[0-9]+]],
// CHECK: ![[NodeInputs_2_1]] = !{![[NodeInput_2_1:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), NodeArray(16) | EmptyRecord(8) | Output(2),
// kDxilNodeMaxRecordsTag(3), 43, kDxilNodeOutputArraySizeTag(5), -1, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_2_1]] = !{i32 1, i32 26, i32 3, i32 43, i32 5, i32 -1, i32 6, i1 true,

// CHECK: = !{void ()* @node_2_2, !"node_2_2", null, null, ![[EntryAttrs_2_2:[0-9]+]]}
// CHECK: ![[EntryAttrs_2_2]] = !{i32 8, i32 15, {{.*}} i32 21, ![[NodeInputs_2_2:[0-9]+]],
// CHECK: ![[NodeInputs_2_2]] = !{![[NodeInput_2_2:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), EmptyRecord(8) | Output(2),
// kDxilNodeMaxRecordsTag(3), 53, kDxilNodeAllowSparseNodesTag(6), true,
// CHECK: ![[NodeInput_2_2]] = !{i32 1, i32 10, i32 3, i32 53,{{.*}} i32 6, i1 true,


struct RECORD1
{
  uint value;
  uint value2;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_0(
    [AllowSparseNodes] [NodeArraySize(129)] [MaxRecords(31)]
    NodeOutputArray<RECORD1> OutputArray_1_0) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_0[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_1(
    [UnboundedSparseNodes] [MaxRecords(37)]
    NodeOutputArray<RECORD1> OutputArray_1_1) {
  ThreadNodeOutputRecords<RECORD1> outRec = OutputArray_1_1[1].GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_1_2(
    [AllowSparseNodes] [MaxRecords(47)]
    NodeOutput<RECORD1> Output_1_2) {
  ThreadNodeOutputRecords<RECORD1> outRec = Output_1_2.GetThreadNodeOutputRecords(2);
  outRec.OutputComplete();
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_0(
    [AllowSparseNodes] [NodeArraySize(131)] [MaxRecords(41)]
    EmptyNodeOutputArray OutputArray_2_0) {
  OutputArray_2_0[1].GroupIncrementOutputCount(10);
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_1(
    [UnboundedSparseNodes] [MaxRecords(43)]
    EmptyNodeOutputArray OutputArray_2_1) {
  OutputArray_2_1[1].GroupIncrementOutputCount(10);
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(1, 1, 1)]
void node_2_2(
    [AllowSparseNodes] [MaxRecords(53)]
    EmptyNodeOutput Output_2_2) {
  Output_2_2.GroupIncrementOutputCount(10);
}
