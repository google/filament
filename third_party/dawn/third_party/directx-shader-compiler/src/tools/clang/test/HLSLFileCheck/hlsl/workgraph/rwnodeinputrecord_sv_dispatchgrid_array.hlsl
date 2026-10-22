// RUN: %dxc -T lib_6_8 %s | FileCheck %s

// Check that SV_DispatchGrid supports array

// CHECK: !dx.entryPoints = !{!{{[0-9+]+}}, ![[node01:[0-9+]+]]}
// CHECK: ![[node01]] = !{void ()* @node01, !"node01", null, null, ![[tags:[0-9+]+]]}
// CHECK: ![[tags]] = !{i32 8, i32 15, i32 13, i32 2, i32 15, !{{[0-9+]+}}, i32 16, i32 -1, i32 20, ![[inputs:[0-9+]+]], i32 4, !{{[0-9+]+}}, i32 5, !{{[0-9+]+}}}
// CHECK: ![[inputs]] = !{![[input0:[0-9+]+]]}
// CHECK: ![[input0]] = !{
// CHECK-SAME: i32 2, ![[recordty:[0-9+]+]]
// CHECK: ![[recordty]] = !{
// CHECK-SAME: i32 1, ![[svdispatchgrid:[0-9+]+]]
// CHECK: ![[svdispatchgrid]] = !{i32 12, i32 5, i32 3}

struct RECORD
{
  uint a[3];
  uint b[3] : SV_DispatchGrid;
};

[Shader("node")]
[NodeLaunch("coalescing")]
[numthreads(4,4,4)]
void node01(RWGroupNodeInputRecords<RECORD> input)
{
  input.Get().a = input.Get().b;
}
