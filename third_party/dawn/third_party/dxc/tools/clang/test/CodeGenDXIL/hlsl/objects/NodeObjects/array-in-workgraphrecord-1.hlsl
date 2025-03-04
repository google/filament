// RUN: %dxc -Tlib_6_8 %s | FileCheck %s

// Make sure generate correct metadata for Entry.

// CHECK: !{void ()* @Entry, !"Entry", null, null, ![[ENTRY:[0-9]+]]}
// CHECK: ![[ENTRY]] = !{i32 8, i32 15, i32 13, i32 1, i32 14, i1 true, i32 15, ![[NodeId:[0-9]+]], i32 16, i32 -1, i32 18, ![[NodeDispatchGrid:[0-9]+]], i32 20, ![[NodeInputs:[0-9]+]], i32 4, ![[NumThreads:[0-9]+]], i32 5, ![[AutoBindingSpace:[0-9]+]]}
// CHECK: ![[NodeId]] = !{!"Entry", i32 0}
// CHECK: ![[NodeDispatchGrid]] = !{i32 1, i32 1, i32 1}
// CHECK: ![[NodeInputs]] = !{![[Input0:[0-9]+]]}
// CHECK: ![[Input0]] = !{i32 1, i32 97, i32 2, ![[NodeRecordType:[0-9]+]]}
// CHECK: ![[NodeRecordType]] = !{i32 0, i32 68, i32 2, i32 4}
// CHECK: ![[NumThreads]] = !{i32 32, i32 1, i32 1}
// CHECK: ![[AutoBindingSpace]] = !{i32 0}

static const int maxPoints = 8;

struct EntryRecord {
    float2 points[maxPoints];
    int    pointCoint;
};

[shader("node")]
[NodeIsProgramEntry]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(32, 1, 1)] 
void Entry(
    uint gtid : SV_GroupThreadId,
    DispatchNodeInputRecord<EntryRecord> inputData
)
{
    EntryRecord input = inputData.Get();

    [[unroll]]
    for (int i = 0; i < 8; ++i) {
        float2 p = input.points[i];
    }
}
