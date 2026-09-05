// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Check that the NodeShareInputOf metadata entry is populated correctly

struct entryRecord
{
    int data0;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
void firstNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
[NodeShareInputOf("firstNode")]
void secondNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(2, 1, 1)]
[NumThreads(1, 1, 1)]
[NodeShareInputOf("firstNode", 3)]
void thirdNode(DispatchNodeInputRecord<entryRecord> inputData)
{ }

// CHECK: !{void ()* @firstNode, !"firstNode", null, null, [[FIRSTATTRS:![0-9]+]]}
// CHECK: [[FIRSTATTRS]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, [[FIRSTNODE:![0-9]+]],
// NodeShareInputOf entry should not be present
// CHECK-NOT: i32 17, {{![0-9]+}}
// CHECK: [[FIRSTNODE]] = !{!"firstNode", i32 0}

// CHECK: !{void ()* @secondNode, !"secondNode", null, null, [[SECONDATTRS:![0-9]+]]}
// CHECK: [[SECONDATTRS]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, [[SECONDNODE:![0-9]+]],
// NodeShareInputOf entry should reference "firstNode"
// CHECK-SAME: i32 17, [[FIRSTNODE]]
// CHECK-SAME: }
// CHECK: [[SECONDNODE]] = !{!"secondNode", i32 0}

// CHECK: !{void ()* @thirdNode, !"thirdNode", null, null, [[THIRDATTRS:![0-9]+]]}
// CHECK: [[THIRDATTRS]] = !{i32 8, i32 15, i32 13, i32 1, i32 15, [[THIRDNODE:![0-9]+]],
// NodeShareInputOf entry should reference "firstNode" index 3
// CHECK-SAME: i32 17, [[FIRSTNODE_3:![0-9]+]]
// CHECK-SAME: }
// CHECK: [[THIRDNODE]] = !{!"thirdNode", i32 0}
// CHECK: [[FIRSTNODE_3]] = !{!"firstNode", i32 3}

