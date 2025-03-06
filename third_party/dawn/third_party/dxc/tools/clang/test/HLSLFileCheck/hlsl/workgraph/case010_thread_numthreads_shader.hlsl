// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// ==================================================================
// Thread launch node may define NumThreads(1,1,1)
// ==================================================================

[Shader("node")]
[NodeLaunch("thread")]
[NumThreads(1,1,1)]
[NodeIsProgramEntry]
void node010_thread_numthreads_shader()
{
}

// CHECK: !{void ()* @node010_thread_numthreads_shader, !"node010_thread_numthreads_shader", null, null, [[ATTR:![0-9]+]]}
// CHECK: [[ATTR]] = !{i32 8, i32 15, i32 13, i32 3,
// CHECK-SAME: i32 4, [[NUMTHREADS:![0-9]+]]
// CHECK: [[NUMTHREADS]] = !{i32 1, i32 1, i32 1}

