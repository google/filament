// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Check the WaveSize attribute is accepted by work graph nodes
// and appears in the metadata

struct INPUT_RECORD
{
  uint DispatchGrid1 : SV_DispatchGrid;
  uint2 a;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(4)]
void node01(DispatchNodeInputRecord<INPUT_RECORD> input) { }

// CHECK: !{void ()* @node01, !"node01", null, null, [[NODE01:![0-9]+]]}
// CHECK: [[NODE01]] = !{i32 8, i32 15, i32 13, i32 1, i32 23, [[NODE01_WS:![0-9]+]]
// CHECK: [[NODE01_WS]] = !{i32 4, i32 0, i32 0}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[WaveSize(8)]
[NodeMaxDispatchGrid(32,1,1)]
void node02(DispatchNodeInputRecord<INPUT_RECORD> input) { }

// CHECK: !{void ()* @node02, !"node02", null, null, [[NODE02:![0-9]+]]}
// CHECK: [[NODE02]] = !{i32 8, i32 15, i32 13, i32 1, i32 23, [[NODE02_WS:![0-9]+]]
// CHECK: [[NODE02_WS]] = !{i32 8, i32 0, i32 0}

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1,1,1)]
[WaveSize(16)]
void node03(RWGroupNodeInputRecords<INPUT_RECORD> input) { }

// CHECK: !{void ()* @node03, !"node03", null, null, [[NODE03:![0-9]+]]}
// CHECK: [[NODE03]] = !{i32 8, i32 15, i32 13, i32 2, i32 23, [[NODE03_WS:![0-9]+]]
// CHECK: [[NODE03_WS]] = !{i32 16, i32 0, i32 0}

[Shader("node")]
[NodeLaunch("thread")]
[WaveSize(32)]
void node04(ThreadNodeInputRecord<INPUT_RECORD> input) { }

// CHECK: !{void ()* @node04, !"node04", null, null, [[NODE04:![0-9]+]]}
// CHECK: [[NODE04]] = !{i32 8, i32 15, i32 13, i32 3, i32 23, [[NODE04_WS:![0-9]+]]
// CHECK: [[NODE04_WS]] = !{i32 32, i32 0, i32 0}
