// RUN: %dxc -T lib_6_8 external %s | FileCheck %s
// ==================================================================
// GetRemainingRecusionLevels() called
// ==================================================================

RWBuffer<uint> buf0;

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(8,1,1)]
[NodeDispatchGrid(32,2,2)]
[NodeMaxRecursionDepth(16)]
void node133_getremainingrecursionlevels()
{
  uint remaining = GetRemainingRecursionLevels();
  // Use resource as a way of preventing DCE
  buf0[0] = remaining;
}

// Shader function
// Arg #1: Opcode = <CreateNodeInputHandle>
// Arg #2: Metadata ID
// ------------------------------------------------------------------
// CHECK: define void @node133_getremainingrecursionlevels()
// CHECK-SAME: {
// CHECK:   {{%[0-9]+}} = call i32 @dx.op.getRemainingRecursionLevels(i32 {{[0-9]+}})  ; GetRemainingRecursionLevels()
// CHECK:   ret void
// CHECK: }

// Metadata for node
// ------------------------------------------------------------------
// CHECK: = !{void ()* @node133_getremainingrecursionlevels, !"node133_getremainingrecursionlevels", null, null, [[ATTRS:![0-9]+]]}

// Metadata for node attributes
// Arg #1: Shader Kind Tag (8)
// Arg #2: Node (15)
// Arg #3: NodeLaunch Tag (13)
// Arg #4: broadcasting (1)
// ...
// Arg #x: NodeMaxrecursionDepth Tag (19)
// Arg #x+1: value (16)
// ...
// ------------------------------------------------------------------
// CHECK: [[ATTRS]] = !{
// CHECK-SAME: i32 8, i32 15, i32 13, i32 1,
// CHECK-SAME: i32 19, i32 16
// CHECK-SAME: }
