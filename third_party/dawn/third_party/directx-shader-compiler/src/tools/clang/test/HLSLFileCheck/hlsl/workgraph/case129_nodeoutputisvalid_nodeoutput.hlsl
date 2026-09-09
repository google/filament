// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// NodeOutputIsValid() is called with NodeOutput
// ==================================================================

RWBuffer<uint> buf0;

struct RECORD
{
  uint value;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node129_nodeoutputisvalid_nodeoutput(NodeOutput<RECORD> output)
{
  buf0[0] = output.IsValid();
}

// Shader function
// Arg #1: Opcode = <CreateNodeInputHandle>
// Arg #2: Metadata ID
// ------------------------------------------------------------------
// CHECK: define void @node129_nodeoutputisvalid_nodeoutput()
// CHECK-SAME: {
// CHECK: [[OUTPUT:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 {{[0-9]+}})  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK: [[ANN_OUTPUT:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OUTPUT]], %dx.types.NodeInfo { i32 6, i32 4 })
// CHECK: {{%[0-9]+}} = call i1 @dx.op.nodeOutputIsValid(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_OUTPUT]])  ; NodeOutputIsValid(output)
// CHECK: ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node129_nodeoutputisvalid_nodeoutput, !"node129_nodeoutputisvalid_nodeoutput", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// Arg #x: NodeOutputs Tag (21)
// Arg #x+1: NodeOutputs (metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1,
// CHECK-SAME: i32 21, [[NODE_OUT:![0-9]+]],
// CHECK-SAME: }

// NodeOutputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: RWOutputRecord (6)
// Arg #3: NodeRecordType Tag (2)
// Arg #4: INPUT_RECORD type
// Arg #5: NodeMaxOutputRecords Tag (4)
// Arg #6: value (0) - TODO: shouldn't this be 1?
// Arg #7: NodeOutputID Tag (0)
// Arg #8: NodeOutput (metadata)
// ------------------------------------------------------------------
// CHECK: [[NODE_OUT]] = !{[[OUTPUT:![0-9]+]]}
// CHECK: [[OUTPUT]] = !{i32 1, i32 6, i32 2, {{![0-9]+}}, i32 3, i32 0, i32 0, [[NODE_ID:![0-9]+]]}

// NodeID
// ------------------------------------------------------------------
// CHECK: !{!"output", i32 0}
