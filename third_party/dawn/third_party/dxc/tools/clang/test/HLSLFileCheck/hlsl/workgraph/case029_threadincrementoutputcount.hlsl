// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// Node with EmptyNodeOutput calls GroupIncrementOutputCount
// ==================================================================


[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node028_incrementoutputcount([MaxRecords(20)] EmptyNodeOutput empty)
{
  empty.ThreadIncrementOutputCount(1);
}

// CHECK: define void @node028_incrementoutputcount()
// CHECK-SAME: {
// CHECK:   [[OP:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.createNodeOutputHandle(i32 {{[0-9]+}}, i32 0) ; CreateNodeOutputHandle(MetadataIdx)
// CHECK:   [[ANN_OP:%[0-9]+]] =  call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[OP]], %dx.types.NodeInfo { i32 10, i32 0 })
// CHECK:   call void @dx.op.incrementOutputCount(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANN_OP]], i32 1, i1 true) ; IncrementOutputCount(output,count,perThread)
// CHECK:   ret void
// CHECK: }

// CHECK: = !{void ()* @node028_incrementoutputcount, !"node028_incrementoutputcount", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: thread (3)
// ...
// Arg #n: NodeOutputs Tag (21)
// Arg #n+1: NodeOutputs (NodeOutputs metadata)
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 3,
// CHECK-SAME: i32 21, [[NODE_OUT:![0-9]+]]
// CHECK-SAME: }

// NodeOutputs
// Arg #1: NodeIOKind Tag (1)
// Arg #2: EmptyOutputRecord (10)
// Arg #3: MaxOutputRecords Tag (4)
// Arg #4: 20 Records Max
// ------------------------------------------------------------------
// CHECK: [[NODE_OUT]] = !{[[OUTPUT0:![0-9]+]]}
// CHECK: [[OUTPUT0]] = !{i32 1, i32 10, i32 3, i32 20
