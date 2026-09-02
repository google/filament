// RUN: %dxc -T lib_6_8 -default-linkage external %s | FileCheck %s
// ==================================================================
// Compute shaders ignore graphnode attributes
// ==================================================================

// Shader functions
// ------------------------------------------------------------------
// CHECK: define void @node051_compute_attrs()
// CHECK-SAME: {
// CHECK:   ret void
// CHECK: }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2,3,2)]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void node051_compute_attrs()
{
}

// Metadata for node
// ------------------------------------------------------------------
// CHECK: !dx.entryPoints = !{
// CHECK-SAME: }
// CHECK: = !{void ()* @node051_compute_attrs, !"node051_compute_attrs", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: NumThreads Tag (4)
// Arg #2: NumThreads (metadata)
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK-SAME: }

// NumThreads
// Arg #1: 1
// Arg #2: 1
// Arg #3: 1
// ------------------------------------------------------------------
// CHECK: [[NUMTHREADS]] = !{i32 1, i32 1, i32 1}
