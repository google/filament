// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// FinishedCrossGroupSharing() is called with RWDispatchNodeInputRecord
// ==================================================================

// Template for input record
// ------------------------------------------------------------------
// CHECK: %dx.types.NodeRecordHandle = type { i8* }

RWBuffer<uint> buf0;

struct [NodeTrackRWInputSharing] INPUT_RECORD
{
  uint value;
};

// Shader function
// Arg #1: Opcode = <createNodeInputRecordHandle>
// Arg #2: Metadata ID
// ------------------------------------------------------------------
// CHECK: define void @node037_finishedcrossgroupsharing()
// CHECK-SAME: {
// CHECK: [[INPUTS:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.createNodeInputRecordHandle(i32 {{[0-9]+}}, i32 0)
// CHECK: [[ANN_INPUTS:%[0-9]+]] = call %dx.types.NodeRecordHandle @dx.op.annotateNodeRecordHandle(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[INPUTS]], %dx.types.NodeRecordInfo { i32 357, i32 4 })
// CHECK: {{%[0-9]+}} = call i1  @dx.op.finishedCrossGroupSharing(i32 {{[0-9]+}}, %dx.types.NodeRecordHandle [[ANN_INPUTS]])
// CHECK: ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node037_finishedcrossgroupsharing(RWDispatchNodeInputRecord<INPUT_RECORD> input)
{
  bool b = input.FinishedCrossGroupSharing();
  buf0[0] = 0 ? b : 1;
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node037_finishedcrossgroupsharing, !"node037_finishedcrossgroupsharing", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// Arg #x: NodeDispatchGrid Tag (18)
// Arg #x+1: NodeDispatchGrid (xyz metadata)
// ...
// Arg #y: NodeInputs Tag (20)
// Arg #y+1: NodeInputs (metadata)
// ...
// Arg #z: NumThreads Tag (4)
// Arg #z+1: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, !{{[0-9]+}}, i32 16, i32 -1, i32 18, [[DISPATCHGRID:![0-9]+]], i32 20, [[NODE_IN:![0-9]+]], i32 4, [[NUMTHREADS:![0-9]+]]

// DispatchGrid
// Arg #1: 256
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[DISPATCHGRID]] = !{i32 256, i32 1, i32 1}

// NodeInputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: RWInputRecord(357)
// Arg #3: NodeRecordType Tag (2)
// Arg #4: INPUT_RECORD type
// ------------------------------------------------------------------
// CHECK: [[NODE_IN]] = !{[[INPUT0:![0-9]+]]}
// CHECK: [[INPUT0]] = !{i32 1, i32 357, i32 2, [[INPUT_RECORD:![0-9]+]]}

// Metadata for input record struct
// Arg #1: Size Tag (0)
// Arg #2: 4 bytes
// ------------------------------------------------------------------
// CHECK: [[INPUT_RECORD]] = !{i32 0, i32 4, i32 2, i32 4}

// NumThreads
// Arg #1: 1024
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK-DAG: [[NUMTHREADS]] = !{i32 1, i32 1, i32 1}
