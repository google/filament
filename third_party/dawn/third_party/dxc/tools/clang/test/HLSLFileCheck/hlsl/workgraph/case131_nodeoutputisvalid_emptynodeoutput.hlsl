// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// NodeOutputIsValid() is called with EmptyNodeOutput
// ==================================================================

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1,1,1)]
void node131_nodeoutputisvalid_emptynodeoutput(EmptyNodeOutput output)
{
  buf0[0] = output.IsValid();
}

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node131_nodeoutputisvalid_emptynodeoutput()
// CHECK-SAME: {
// CHECK: [[OUTPUT:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 {{[0-9]+}})  ; CreateNodeOutputHandle(MetadataIdx)
// CHECK: [[ANN_OUTPUT:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OUTPUT]], %dx.types.NodeInfo { i32 10, i32 0 })
// CHECK: {{%[0-9]+}} = call i1 @dx.op.nodeOutputIsValid(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_OUTPUT]])  ; NodeOutputIsValid(output)
// CHECK: ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node131_nodeoutputisvalid_emptynodeoutput, !"node131_nodeoutputisvalid_emptynodeoutput", null, null, [[ATTRS:![0-9]+]]}

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
// Arg #2: EmptyNodeOutput (10)
// Arg #3: NodeMaxOuputRecords Tag (4)
// Arg #4: value (0)
// Arg #5: NodeOutputID Tag (0)
// Arg #6: NodeID (metadata)
// ------------------------------------------------------------------
// CHECK: [[NODE_OUT]] = !{[[OUTPUT:![0-9]+]]}
// CHECK: [[OUTPUT]] = !{i32 1, i32 10, i32 3, i32 0, i32 0, [[NODE_ID:![0-9]+]]}

// NodeID
// ------------------------------------------------------------------
// CHECK: !{!"output", i32 0}
