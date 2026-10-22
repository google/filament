// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// If no input is specified then the NodeInputs metadata should not
// be present.
// ==================================================================

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeDispatchGrid(4,1,1)]
void node070_broadcasting() { }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
void node070_coalescing() { }

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
void node070_thread() { }


// Metadata for node070_broadcasting
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node070_broadcasting, !"node070_broadcasting", null, null, [[ATTRS_BROADCASTING:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS_BROADCASTING]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1,
// CHECK-NOT: i32 20, i32 {{![-=9]+}}
// CHECK-SAME: }

// Metadata for node070_coalescing
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node070_coalescing, !"node070_coalescing", null, null, [[ATTRS_COALESCING:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: coalescing (2)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS_COALESCING]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 2,
// CHECK-NOT: i32 20, i32 {{![-=9]+}}
// CHECK-SAME: }

// Metadata for node070_thread
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node070_thread, !"node070_thread", null, null, [[ATTRS_THREAD:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: thread (3)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS_THREAD]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 3,
// CHECK-NOT: i32 20, i32 {{![-=9]+}}
// CHECK-SAME: }
