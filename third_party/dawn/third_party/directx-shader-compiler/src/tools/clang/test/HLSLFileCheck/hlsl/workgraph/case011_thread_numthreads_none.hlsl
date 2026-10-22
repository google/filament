// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Thread launch node without NumThreads specified should use a
// default of (1,1,1)
// ==================================================================

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node011_thread_numthreads_none()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("thread")]
[NodeIsProgramEntry]
void node011_thread_numthreads_none()
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node011_thread_numthreads_none, !"node011_thread_numthreads_none", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: thread (3)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{i32 8, i32 15, i32 13, i32 3,
// CHECK-SAME: i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }
// CHECK: [[NUMTHREADS]] = !{i32 1, i32 1, i32 1}
