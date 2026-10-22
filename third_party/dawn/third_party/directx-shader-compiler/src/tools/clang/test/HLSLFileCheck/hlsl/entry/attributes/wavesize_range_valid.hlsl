// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Check the WaveSize attribute emits the right metadata in DXIL

struct INPUT_RECORD
{
  uint DispatchGrid1 : SV_DispatchGrid;
  uint2 a;
};


// CHECK: @node01, !"node01", null, null, [[PROPS:![0-9]+]]}
// CHECK: [[PROPS]] = !{i32 8, i32 15, i32 13, i32 1, i32 23, [[WS:![0-9]+]]
// CHECK: [[WS]] = !{i32 4, i32 16, i32 8}

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(4, 16, 8)]
void node01(DispatchNodeInputRecord<INPUT_RECORD> input) { }

// CHECK: @node02, !"node02", null, null, [[PROPS:![0-9]+]]}
// CHECK: [[PROPS]] = !{i32 8, i32 15, i32 13, i32 1, i32 23, [[WS:![0-9]+]]
// CHECK: [[WS]] = !{i32 4, i32 16, i32 0}
[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(4, 16)]
void node02(DispatchNodeInputRecord<INPUT_RECORD> input) { }
