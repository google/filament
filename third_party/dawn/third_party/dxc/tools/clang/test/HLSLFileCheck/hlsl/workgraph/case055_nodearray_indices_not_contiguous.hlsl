// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// CASE055
// Node array with non-contiguous indices
// ==================================================================

// Shader functions
// ------------------------------------------------------------------
// CHECK: define void @node055a_nodearray_indices_not_contiguous()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1024,1,1)]
[NodeID("node_array")]
[NodeIsProgramEntry]
void node055a_nodearray_indices_not_contiguous()
{
}

// CHECK: define void @node055b_nodearray_indices_not_contiguous()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(256,1,1)]
[NumThreads(1024,1,1)]
[NodeID("node_array", 5)]
[NodeIsProgramEntry]
void node055b_nodearray_indices_not_contiguous()
{
}

// Metadata for node 0
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{[[ENTRYX:![0-9]+]], [[ENTRY0:![0-9]+]], [[ENTRY1:![0-9]+]]
// CHECK-SAME: }
// CHECK: [[ENTRY0]] = !{void ()* @node055a_nodearray_indices_not_contiguous, !"node055a_nodearray_indices_not_contiguous", null, null, [[ATTRS0:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// Arg #7: NodeId Tag (15)
// Arg #8: NodeId (NodeId metadata)
// Arg #9: NodeLocalRootArgumentsTableIndex Tag (16)
// Arg #10: Index (-1)
// Arg #11: NodeDispatchGrid Tag (18)
// Arg #12: NodeDispatchGrid (xyz metadata)
// Arg #13: NumThreads Tag (4)
// Arg #14: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS0]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1, i32 14, i1 true, i32 15, [[NODEID0:![0-9]+]], i32 16, i32 -1, i32 18, [[DISPATCHGRID:![0-9]+]], i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }

// NodeID
// Arg #1: NodeID = "node_array"
// Arg #2: Default Index (0)
// ------------------------------------------------------------------
// CHECK: [[NODEID0]] = !{!"node_array", i32 0}

// DispatchGrid
// Arg #1: 256
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[DISPATCHGRID]] = !{i32 256, i32 1, i32 1}

// NumThreads
// Arg #1: 1024
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[NUMTHREADS]] = !{i32 1024, i32 1, i32 1}

// Metadata for node 1
// ------------------------------------------------------------------
// CHECK: [[ENTRY1]] = !{void ()* @node055b_nodearray_indices_not_contiguous, !"node055b_nodearray_indices_not_contiguous", null, null, [[ATTRS1:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// Arg #7: NodeId Tag (15)
// Arg #8: NodeId (NodeId metadata)
// Arg #9: NodeLocalRootArgumentsTableIndex Tag (16)
// Arg #10: Index (-1)
// Arg #11: NodeDispatchGrid Tag (18)
// Arg #12: NodeDispatchGrid (xyz metadata)
// Arg #13: NumThreads Tag (4)
// Arg #14: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS1]] = !{

// CHECK-SAME: i32 8, i32 15, i32 13, i32 1, i32 14, i1 true, i32 15, [[NODEID1:![0-9]+]], i32 16, i32 -1, i32 18, [[DISPATCHGRID:![0-9]+]], i32 4, [[NUMTHREADS]]
// CHECK-SAME: }

// NodeID
// Arg #1: NodeID = "node_array"
// Arg #2: Index = 5
// ------------------------------------------------------------------
// CHECK-DAG: [[NODEID1]] = !{!"node_array", i32 5}
