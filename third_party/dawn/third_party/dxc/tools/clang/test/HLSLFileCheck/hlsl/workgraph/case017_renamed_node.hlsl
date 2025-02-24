// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// Renamed node, unnamed index defaults to 0
// ==================================================================

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node017_renamed_node()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("thread")]
[NodeID("new_node_name")]
[NodeIsProgramEntry]
void node017_renamed_node()
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node017_renamed_node, !"node017_renamed_node", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: thread (3)
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// Arg #7: NodeId Tag (15)
// Arg #8: NodeId (NodeId metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 3, i32 14, i1 true, i32 15, [[NODEID:![0-9]+]]
// CHECK-SAME: }

// NodeID
// Arg #1: NodeID = "new_node_name"
// Arg #2: Default Index (0)
// ------------------------------------------------------------------
// CHECK: [[NODEID]] = !{!"new_node_name", i32 0}
