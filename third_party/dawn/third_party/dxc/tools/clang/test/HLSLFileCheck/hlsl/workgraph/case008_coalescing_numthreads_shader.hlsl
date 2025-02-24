// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// Coalescing launch node with thread group defined in the shader
// ==================================================================

// Shader function
// ------------------------------------------------------------------
// CHECK: define void @node008_coalescing_numthreads_shader()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
[NodeIsProgramEntry]
void node008_coalescing_numthreads_shader()
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node008_coalescing_numthreads_shader, !"node008_coalescing_numthreads_shader", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: ShaderKind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: coalescing (2)
// Arg #5: NodeIsProgramEntry Tag (14)
// Arg #6: True (1)
// Arg #7: NodeId Tag (15)
// Arg #8: NodeId (NodeId metadata)
// Arg #9: NodeLocalRootArgumentsTableIndex Tag (16)
// Arg #10: Index (-1)
// ...
// Arg #n1: NumThreads Tag (4)
// Arg #n2: NumThreads (xyz metadata)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 2, i32 14, i1 true, i32 15, [[NODEID:![0-9]+]], i32 16, i32 -1,
// CHECK-SAME: i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }

// NumThreads
// Arg #1: 1024
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[NUMTHREADS]] = !{i32 1024, i32 1, i32 1}
