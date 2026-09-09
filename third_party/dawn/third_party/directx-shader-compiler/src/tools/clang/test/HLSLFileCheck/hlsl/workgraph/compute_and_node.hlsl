// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Test when permutations of compute and node are specified:
// - Check that Shader Kind is compute whenever compute is specified
// - Check that NodeId is present when and only when node is specified.
// - Check that NumThreads is present and correctly populated in all cases
// ==================================================================


// ==================================================================

[Shader("compute")]
[NumThreads(9,3,4)]
void compute_only() { }

// Only compute specified: Shader Kind = compute, NodeId not present
// CHECK: !{void ()* @compute_only, !"compute_only", null, null, [[ATTRS_2:![0-9]+]]}
// CHECK: [[ATTRS_2]] =  !{i32 8, i32 5,
// CHECK-NOT: i32 15, {{![0-9]+}},
// CHECK-SAME: i32 4, [[NUM_THREADS_2:![0-9]+]],
// CHECK: [[NUM_THREADS_2]] = !{i32 9, i32 3, i32 4}

// ==================================================================

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(9,7,8)]
void node_only() { }

// Only node specified: Shader Kind = node, NodeId present
// CHECK: !{void ()* @node_only, !"node_only", null, null, [[ATTRS_4:![0-9]+]]}
// CHECK: [[ATTRS_4]] =  !{i32 8, i32 15,
// CHECK-SAME: i32 15, [[NODE_ID_4:![0-9]+]],
// CHECK-SAME: i32 4, [[NUM_THREADS_4:![0-9]+]],
// CHECK: [[NODE_ID_4]] = !{!"node_only", i32 0}
// CHECK: [[NUM_THREADS_4]] = !{i32 9, i32 7, i32 8}
