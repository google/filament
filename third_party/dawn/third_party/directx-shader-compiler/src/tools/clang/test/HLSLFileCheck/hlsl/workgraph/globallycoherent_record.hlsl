// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s -check-prefix=AST
// RUN: %dxc -T lib_6_8 -fcgl %s | FileCheck %s -check-prefix=HLCHECK
// RUN: %dxc -T lib_6_8 %s | %D3DReflect %s | FileCheck %s -check-prefix=REFLECT
// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od -Zi %s | FileCheck %s
// ==================================================================
// Check globallycoherent modifier on RWDispatchNodeInputRecord
// ==================================================================

// ==== AST Checks ====

// AST: FunctionDecl {{.*}} used DoIt
// AST: ParmVarDecl {{.*}} used funcInputData 'globallycoherent RWDispatchNodeInputRecord<Record>':'RWDispatchNodeInputRecord<Record>'
// AST-NEXT: HLSLGloballyCoherentAttr
// AST: FunctionDecl {{.*}} firstNode
// AST: ParmVarDecl {{.*}} used inputData 'globallycoherent RWDispatchNodeInputRecord<Record>':'RWDispatchNodeInputRecord<Record>'
// AST-NEXT: HLSLGloballyCoherentAttr

// ==== -fcgl Checks ====

// TBD: We should be annotating handles used inside called functions
// xHLCHECK: define internal void @"\01?DoIt
// xHLCHECK: %[[FuncAnnInputRecHandle:[0-9]+]] = call %dx.types.NodeRecordHandle @"dx.hl.annotatenoderecordhandle..%dx.types.NodeRecordHandle (i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo)"(i32 16, %dx.types.NodeRecordHandle %[[FuncInputRecHandle]], %dx.types.NodeRecordInfo { i32 613, i32 4 })

// HLCHECK: define void @firstNode
// HLCHECK: %[[CreateInputHandle:[0-9]+]] = call %dx.types.NodeRecordHandle @"dx.hl.createnodeinputrecordhandle..%dx.types.NodeRecordHandle (i32, i32)"(i32 13, i32 0)

// Check that NodeIOFlags is 613 = GloballyCoherent(512) | DispatchRecord(96) | ReadWrite(4) | Input(1)
// HLCHECK: %[[AnnInputRecHandle:[0-9]+]] = call %dx.types.NodeRecordHandle @"dx.hl.annotatenoderecordhandle..%dx.types.NodeRecordHandle (i32, %dx.types.NodeRecordHandle, %dx.types.NodeRecordInfo)"(i32 16, %dx.types.NodeRecordHandle %[[CreateInputHandle]], %dx.types.NodeRecordInfo { i32 613, i32 4 })

// ==== D3DReflect Checks ====

// REFLECT: Inputs: <{{[0-9]+}}:RecordArrayRef<IONode>[1]> = {
// REFLECT-NEXT: [0]:
// REFLECT-NEXT: IOFlagsAndKind: 613

// ==== DXIL Checks ====

// CHECK-NOT: error
// CHECK: define void @firstNode()
// CHECK:   %[[CreateInputHandle:[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 250, i32 0)

// Check that NodeIOFlags is 613 = GloballyCoherent(512) | DispatchRecord(96) | ReadWrite(4) | Input(1)
// CHECK:   call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 251, %dx.types.NodeRecordHandle %[[CreateInputHandle]], %dx.types.NodeRecordInfo { i32 613, i32 4 })

// ==== DXIL Metadata Checks ====

// CHECK: !{void ()* @firstNode, !"firstNode", null, null, ![[EntryProps:[0-9]+]]}
// kDxilNodeInputsTag(20)
// CHECK: ![[EntryProps]] = {{.*}} i32 20, ![[NodeInputs:[0-9]+]]
// CHECK: ![[NodeInputs]] = !{![[NodeInput:[0-9]+]]}
// kDxilNodeIOFlagsTag(1), 613 = GloballyCoherent(512) | DispatchRecord(96) | ReadWrite(4) | Input(1)
// CHECK: ![[NodeInput]] = !{i32 1, i32 613,

struct Record {
  uint index;
};

void DoIt(
    uint gi,
    globallycoherent RWDispatchNodeInputRecord<Record> funcInputData,
    NodeOutput<Record> outputData) {
  GroupNodeOutputRecords<Record> outputRecord =
      outputData.GetGroupNodeOutputRecords(1);
  if (gi == 0) {
    outputRecord.Get().index = funcInputData.Get().index;
  }
}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1,1,1)]
void firstNode(
    uint gi : SV_GroupIndex,
    globallycoherent RWDispatchNodeInputRecord<Record> inputData,
    NodeOutput<Record> outputData) {
  DoIt(gi, inputData, outputData);
}
