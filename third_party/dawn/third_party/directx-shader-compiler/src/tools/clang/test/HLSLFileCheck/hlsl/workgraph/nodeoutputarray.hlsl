// RUN: %dxc -T lib_6_8 %s | FileCheck %s
// RUN: %dxc -T lib_6_8 -Od %s | FileCheck %s -check-prefixes=CHECK,CHECKOD
// Tests for NodeOutputArray/EmptyNodeOutputArray/IndexNodeHandle
struct RECORD1
{
    uint value;
    uint value2;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_1_0(
    [NodeArraySize(128)] [MaxRecords(64)] NodeOutputArray<RECORD1> OutputArray
)
{
}
// CHECK-LABEL: define void @node_1_0()
// CHECK: ret void

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_1_1(
    [NodeArraySize(128)] [MaxRecords(64)] NodeOutputArray<RECORD1> OutputArray
)
{
    ThreadNodeOutputRecords<RECORD1> outRec = OutputArray[1].GetThreadNodeOutputRecords(2);
    outRec.OutputComplete();
}
// CHECK-LABEL: define void @node_1_1()
// CHECK: [[IDXNH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.indexNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle {{%[0-9]+}}, i32 1)  ; IndexNodeHandle(NodeOutputHandle,ArrayIndex)
// CHECK: [[ANH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[IDXNH]], %dx.types.NodeInfo { i32 22, i32 8 })
// CHECK: call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANH]], i32 2, i1 true)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_1_2(
    [NodeArraySize(128)] [MaxRecords(64)] NodeOutputArray<RECORD1> OutputArray
)
{
    GroupNodeOutputRecords<RECORD1> outRec = OutputArray[1].GetGroupNodeOutputRecords(2);
    outRec.OutputComplete();
}
// CHECK-LABEL: define void @node_1_2()
// CHECK: [[IDXNH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.indexNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle {{%[0-9]+}}, i32 1)  ; IndexNodeHandle(NodeOutputHandle,ArrayIndex)
// CHECK: [[ANH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[IDXNH]], %dx.types.NodeInfo { i32 22, i32 8 })
// CHECK: call %dx.types.NodeRecordHandle @dx.op.allocateNodeOutputRecords(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANH]], i32 2, i1 false)

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_1_3(
    [NodeArraySize(128)] [MaxRecords(64)] NodeOutputArray<RECORD1> OutputArray
)
{
	bool b = OutputArray[1].IsValid();
	if (b) {
		GroupNodeOutputRecords<RECORD1> outRec = OutputArray[1].GetGroupNodeOutputRecords(2);
    	outRec.OutputComplete();
	}
}
// CHECK-LABEL: define void @node_1_3()
// CHECK: [[IDXNH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.indexNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle {{%[0-9]+}}, i32 1)
// CHECK: [[ANH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[IDXNH]], %dx.types.NodeInfo { i32 22, i32 8 })
// CHECK: call i1 @dx.op.nodeOutputIsValid(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANH]])

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_2_0(
    [NodeArraySize(128)] [MaxRecords(64)] EmptyNodeOutputArray EmptyOutputArray
)
{
}
// CHECK-LABEL: define void @node_2_0()
// CHECK: ret void

[Shader("node")]
[NodeLaunch("broadcasting")]
[NodeDispatchGrid(1, 1, 1)]
[NumThreads(128, 1, 1)]
void node_2_1(
    [NodeArraySize(128)] [MaxRecords(64)] EmptyNodeOutputArray EmptyOutputArray
)
{
	bool b = EmptyOutputArray[1].IsValid();
	if (b) {
		EmptyOutputArray[1].GroupIncrementOutputCount(10);
	}
}
// CHECK-LABEL: define void @node_2_1()
// CHECK: [[IDXNH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.indexNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle {{%[0-9]+}}, i32 1)
// CHECK: [[ANH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[IDXNH]], %dx.types.NodeInfo { i32 26, i32 0 })
// CHECK: call i1 @dx.op.nodeOutputIsValid(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANH]])
// CHECKOD-LABEL: {{if\.then|\<label\>}}:
// CHECKOD: [[IDXNH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.indexNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle {{%[0-9]+}}, i32 1)
// CHECKOD: [[ANH:%[0-9]+]] = call %dx.types.NodeHandle @dx.op.annotateNodeHandle(i32 {{[0-9]+}}, %dx.types.NodeHandle [[IDXNH]], %dx.types.NodeInfo { i32 26, i32 0 })
// CHECK: call void @dx.op.incrementOutputCount(i32 {{[0-9]+}}, %dx.types.NodeHandle [[ANH]], i32 10, i1 false)