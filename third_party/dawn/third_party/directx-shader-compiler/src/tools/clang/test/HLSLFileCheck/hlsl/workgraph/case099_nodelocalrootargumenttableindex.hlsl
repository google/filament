// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// NodeLocalRootArgumentsTableIndex is declared
// ==================================================================

// Shader function
// Arg #1: Opcode = <CreateInputRecordHandle>
// Arg #2: Metadata ID = 0
// ------------------------------------------------------------------
// CHECK: define void @node099_localrootargumentstableindex()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("thread")]
[NodeLocalRootArgumentsTableIndex(5)]
[NodeIsProgramEntry]
void node099_localrootargumentstableindex()
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node099_localrootargumentstableindex, !"node099_localrootargumentstableindex", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: NodeLaunch Tag (13)
// Arg #2: thread (3)
// Arg #3: NodeLocalRootArgumentsTableIndex Tag (16)
// Arg #4: Index 5
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 13, i32 3, i32 14, i1 true, i32 15, {{![0-9]+}}, i32 16, i32 5
// CHECK-SAME: }
